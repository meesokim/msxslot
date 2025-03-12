//-----------------------------------------------------------------------------
// MSX BUS Master Controller (MAX II CPLD)
//-----------------------------------------------------------------------------
//
// Description:
//   - Implementation of MSX Bus Master Controller
//   - Supports memory and I/O read/write operations
//   - Controls slot selection and chip select signals
//   - 8bit half bidirectional bit banging protocol (8bit, CLK, CS)
//
// Supported 4bit Commands:
//   - 0x0: Memory Read
//   - 0x1: Memory Write
//   - 0x2: I/O Read
//   - 0x3: I/O Write
//   - 0x5: Reset
//   - 0x8: Status Read
//
// Command Flags (7th bit, 4th bit):
//   - 4th bit: SLTSL1(0) or SLTSL2(1)
//   - 7th bit: M1
//
// State Machine:
//   - IDLE: Waiting state
//   - GET_CMD: Command reception
//   - GET_ADDR_L/H: Address reception (Low/High)
//   - SET_CONTROL: Control signal setup
//   - DELAY_STATE: Delay state
//   - WAIT_STATE: Wait for WAIT signal
//   - GET_DATA: Data reception
//   - GET_STATUS: Read status information
//   - COMPLETE: Operation complete
//
// Slot/Chip Selection:
//   - SLTSL1/SLTSL2: Slot selection
//   - CS1/CS2/CS12: Memory area selection (4000-7FFF/8000-BFFF)
//
// Memory Map:
//   - CS1:  4000h-7FFFh
//   - CS2:  8000h-BFFFh
//   - CS12: Combined CS1 and CS2 areas
//
// Revision History:
//   - Version 1.0 (2017-02-03): Initial Release
//   - Version 2.0 (2025-03-05): Ehanced Implementation with no-standard 8bit-SPI 
//-----------------------------------------------------------------------------
//
//	LOGIC:		MAX II MSX BUS Master
//	MODULE NAME: 	MSX BUS
//	FILE NAME:		msxbus_simple.v
//	COMPANY:		Copyright 2017 Miso Kim. https://www.github.com/meesokim
//
//  This module is the top level module for MAX II MSX BUS Master.
//
// Modify port declaration
module msxbus_simple (
    input CS,
    input CLK,
    input PCLK,
    inout [15:0] RDATA,
    input [1:0] SW,
    input INT, BUSDIR, WAIT,
    inout [7:0] DATA,
    output reg RD,
    output reg WR,
    output reg MREQ,
    output reg IORQ,
    output reg RESET,
    output reg [15:0] ADDR,
    output reg SLTSL1,
    output reg SLTSL2,
    output reg SLTSL1_CS1,    // Changed from CS1
    output reg SLTSL1_CS2,    // Changed from CS2
    output reg SLTSL1_CS12,   // Changed from CS12
    output reg SLTSL2_CS1,    // New
    output reg SLTSL2_CS2,    // New
    output reg SLTSL2_CS12,   // New
    output reg CS1,
    output reg CS2,
    output reg CS12,
    output MCLK, SWOUT, RFSH,
    output reg M1
);

// Add CMD register to the reg declarations
reg [7:0] input_data;
reg [15:0] rdata_out;
reg [7:0] CMD;        // Added CMD register
reg [4:0] state;
reg rdata_drive;
reg [15:0] address_out;

// Add data_drive register declaration
reg [7:0] data_out;
reg data_drive;

// Add bidirectional control for DATA bus
assign RDATA = rdata_drive ? rdata_out : 16'bZ;
assign DATA = data_drive ? data_out : 8'bZ;
assign SWOUT = 1'b1;

// Add to register declarations
reg [1:0] delay_count;

// Add new state to localparam
localparam 
    IDLE = 3'd0,
    GET_ADDR = 3'd1,
    GET_CMD_CTRL = 3'd2,
    GET_STATE_DATA = 3'd3,
    COMPLETE = 4'd9,
    CMD_MEM_READ = 4'd0,
    CMD_MEM_WRITE = 4'd1,
    CMD_IO_READ = 4'd2,
    CMD_IO_WRITE = 4'd3,
    CMD_RESET = 4'd5,
    CMD_STATUS = 4'd8;
// External CLK input directly
reg [7:0] clk_divider;
reg mclk_out;
// Clock divider using external CLK
always @(posedge CLK) begin
    if (clk_divider == 8'd13) begin  // Divide by 14 (50MHz/14 ≈ 3.571MHz)
        clk_divider <= 8'd0;
        mclk_out <= ~mclk_out;
    end else begin
        clk_divider <= clk_divider + 1'b1;
    end
end
assign MCLK = mclk_out;

// Add setup/hold time control
reg [3:0] setup_time;

// Modify always block to use only positive edge of CS
always @(negedge PCLK or posedge CS) begin
    if (CS) begin
        state <= GET_ADDR;
        RD <= 1'b1;
        WR <= 1'b1;
        MREQ <= 1'b1;
        IORQ <= 1'b1;
        SLTSL1 <= 1'b1;    // Added
        SLTSL2 <= 1'b1;    // Added
		SLTSL1_CS1 = 1'b1;
		SLTSL1_CS2 = 1'b1;
		SLTSL1_CS12 = 1'b1;
		SLTSL2_CS1 = 1'b1;
		SLTSL2_CS2 = 1'b1;
		SLTSL2_CS12 = 1'b1;
		M1 = 1'b1;
        rdata_drive <= 1'b0;
		data_drive <= 1'b0;
    end else begin
        case (state)
            GET_ADDR: begin
                address_out <= RDATA;
                state <= GET_CMD_CTRL;
            end

            GET_CMD_CTRL: begin
                CMD <= RDATA[15:8];
                ADDR <= address_out;
                if (CMD[3:0] < 4) begin
                    // Common control signals
                    MREQ <= CMD[1];
                    IORQ <= !CMD[1];
                    RD <= CMD[0];    
                    WR <= !CMD[0];     
                    M1 <= CMD[7];
                    // Memory access signals
                    if (!CMD[1]) begin  // If MREQ is active
                        if (!CMD[5]) begin // If NO_SLTSL
                            SLTSL1 <= CMD[4];
                            SLTSL2 <= !CMD[4];
                            if (!CMD[4]) begin  // SLTSL1 active
                                SLTSL1_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                                SLTSL1_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                                SLTSL1_CS12 <= SLTSL1_CS1 & SLTSL1_CS2;
                            end else begin     // SLTSL2 active
                                SLTSL2_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                                SLTSL2_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                                SLTSL2_CS12 <= SLTSL2_CS1 & SLTSL2_CS2;
                            end
                        end
                    end
                    // State transition and data direction
                    if (CMD[0]) begin  // Write operations
                        data_out <= RDATA[7:0];
                    end
                    data_drive <= CMD[0];
                    rdata_drive <= 1'b1;
                    rdata_out <= 16'h0000;
                    state <= GET_STATE_DATA;
                end
                else begin
                    case (CMD[3:0])    // Only use lower 4 bits for command
                        CMD_RESET: begin // Reset
                            RESET <= CMD[4];
                            state <= IDLE;
                        end
                        CMD_STATUS: begin // Get Status
                            state <= GET_STATE_DATA;
                            rdata_drive <= 1'b1;
                        end
                        default: state <= IDLE;
                    endcase
                end
            end

            GET_STATE_DATA: begin
                rdata_out <= {WAIT, INT, SW, 4'b00, DATA};
                end
            end

        endcase
    end
end

// Remove the old combinational chip select logic block
endmodule



