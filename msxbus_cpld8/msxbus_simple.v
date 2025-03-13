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
    inout [7:0] RDATA,
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
    output MCLK, SWOUT, RFSH,
    output reg M1
);

// Add CMD register to the reg declarations
reg [7:0] input_data;
reg [7:0] rdata_out;
reg [7:0] CMD;        // Added CMD register
reg [4:0] state;
reg rdata_drive;

// Add data_drive register declaration
reg [7:0] data_out;
reg data_drive;

// Add bidirectional control for DATA bus
assign RDATA = rdata_drive ? rdata_out : 8'bZ;
assign DATA = data_drive ? data_out : 8'bZ;
assign SWOUT = 1'b1;

// Add to register declarations
reg [1:0] delay_count;

// Add new state to localparam
localparam 
    IDLE = 3'd0,
    GET_CMD = 3'd1,
    GET_ADDR_L = 3'd2,
    GET_ADDR_H = 3'd3,
    SET_CONTROL = 3'd4,
    SEND_DATA = 3'd5,
    GET_DATA = 3'd6,
    WAIT_STATE = 3'd7,
    GET_STATUS = 3'd8,    // New state
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

// Modify always block to use only positive edge of CS
always @(negedge PCLK or posedge CS) begin
    if (CS) begin
        state <= GET_CMD;
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
        RESET <= 1'b1;
        rdata_drive <= 1'b0;
		data_drive <= 1'b0;
        data_out <= 8'b11111111;
    end else begin
        case (state)
            GET_CMD: begin
                CMD <= RDATA;
                case (RDATA[3:0])    // Only use lower 4 bits for command
                    CMD_MEM_READ: begin // Memory Read
                        state <= GET_ADDR_L;
                    end
                    CMD_MEM_WRITE: begin // Memory Write
                        state <= SEND_DATA;
                    end
                    CMD_IO_READ: begin // IO Read
                        state <= GET_ADDR_L;
                    end
                    CMD_IO_WRITE: begin // IO Write
                        state <= SEND_DATA;
                    end
                    CMD_RESET: begin // Reset
                        RESET <= 1'b0;
                        state <= IDLE;
                    end
                    CMD_STATUS: begin // Get Status
                        state <= GET_STATUS;
                        rdata_drive <= 1'b1;
                    end
                    default: state <= IDLE;
                endcase
            end

            SEND_DATA: begin
                data_out <= RDATA;
                data_drive <= CMD[0];    
                state <= GET_ADDR_L;
            end

            GET_ADDR_L: begin
                ADDR[7:0] <= RDATA;
                state <= GET_ADDR_H;
            end

            GET_ADDR_H: begin
                ADDR[15:8] <= RDATA;
                // Common control signals
                MREQ <= CMD[1];
                IORQ <= !CMD[1];
                RD <= CMD[0];    
                WR <= !CMD[0];     
                // Memory access signals
                if (!CMD[1]) begin  // If MREQ is active
                    if (!CMD[5]) begin // If NO_SLTSL
                        SLTSL1 <= CMD[4];
                        SLTSL2 <= !CMD[4];
                    end
                    if (!CMD[4]) begin  // SLTSL1 active
                        SLTSL1_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                        SLTSL1_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                        SLTSL1_CS12 <= ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10)) ? 1'b0 : 1'b1;
                    end else begin     // SLTSL2 active
                        SLTSL2_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                        SLTSL2_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                        SLTSL2_CS12 <= ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10)) ? 1'b0 : 1'b1;
                    end
                end
                // State transition and data direction
                rdata_drive <= 1'b1;
				rdata_out <= 8'h00;
                state <= WAIT_STATE;
            end

            WAIT_STATE: begin
                if (WAIT) begin
                    rdata_out <= 8'hFF; // ACK
					if (!CMD[0]) begin
	                    state <= GET_DATA;
					end else begin
	                    state <= COMPLETE;
					end
                end
            end

            GET_DATA: begin
                rdata_out <= DATA;
                state <= COMPLETE;
            end

            GET_STATUS: begin
                rdata_out <= {INT, SW, 6'b0000, WAIT};  // INT in bit 7, SW[1:0] in bit 6-5, WAIT in bit 0
                state <= COMPLETE;
            end

            COMPLETE: begin
                state <= IDLE;
            end
        endcase
    end
end

// Remove the old combinational chip select logic block
endmodule

