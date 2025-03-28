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

/* verilator lint_off CASEINCOMPLETE */

// Modify port declaration
module msxbus_simple (
    input CS,
    input CLK,
    input PCLK,
    input RELEASE,
    inout [7:0] RDATA,
    input [1:0] SW,
    input INT, BUSDIR, WAIT,    // BUSDIR back to input
    inout reg [7:0] DATA,
    output reg RD,
    output reg WR,
    output reg MREQ,
    output reg IORQ,
    output reg RESET,
    output RWAIT,
    output reg [15:0] ADDR,
    output reg SLTSL1,
    output reg SLTSL2,
    output SLTSL1_CS1,    // Changed from CS1
    output SLTSL1_CS2,    // Changed from CS2
    output SLTSL1_CS12,   // Changed from CS12
    output SLTSL2_CS1,    // New
    output SLTSL2_CS2,    // New
    output SLTSL2_CS12,   // New
    output MCLK, SWOUT, RFSH,
    output reg M1
);

// Add CMD register to the reg declarations
reg [7:0] data;
reg [7:0] rdata_out;
reg [7:0] cmd;        // Added CMD register
reg [3:0] state;
reg rdata_drive;

// Add data_drive register declaration
reg [7:0] data_out;
reg data_drive;

// Add bidirectional control for DATA bus
assign RDATA = rdata_drive ? rdata_out : 8'bZ;
// Modify DATA bus control based on RD/WR signals
// assign DATA = data_drive ? data_out : 8'bZ;
assign RDATA = rdata_drive ? rdata_out : 8'bZ;
assign SWOUT = 1'b1;
assign SLTSL1_CS1 = !SLTSL1 && (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
assign SLTSL1_CS2 = !SLTSL1 && (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
assign SLTSL1_CS12 = (!SLTSL1 && ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10))) ? 1'b0 : 1'b1;
assign SLTSL2_CS1 = !SLTSL2 && (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
assign SLTSL2_CS2 = !SLTSL2 && (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
assign SLTSL2_CS12 = (!SLTSL2 && ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10))) ? 1'b0 : 1'b1;
assign RWAIT = WAIT;

// Add new state to localparam
localparam 
    IDLE = 4'd0,
    GET_CMD = 4'd1,
    GET_ADDR_L = 4'd2,
    GET_ADDR_H = 4'd3,
    WRITE_DATA = 4'd4,
    WRITE_MSXBUS = 4'd5,
    READ_MSXBUS = 4'd6,
    WAIT_STATE = 4'd7,
    GET_STATUS = 4'd8,    // New state
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
    if (clk_divider == 8'd13) begin  // Divide by 14 (50MHz/14 ? 3.571MHz)
        clk_divider <= 8'd0;
        mclk_out <= ~mclk_out;
    end else begin
        clk_divider <= clk_divider + 1'b1;
    end
end
assign MCLK = mclk_out;

// Add initial values for registers
initial begin
    state = IDLE;
    RD = 1'b1;
    WR = 1'b1;
    MREQ = 1'b1;
    IORQ = 1'b1;
    SLTSL1 = 1'b1;
    SLTSL2 = 1'b1;
    M1 = 1'b1;
    RESET = 1'b1;
    rdata_drive = 1'b0;
    data_drive = 1'b0;
    data_out = 8'hFF;
    rdata_out = 8'hFF;
    clk_divider = 8'd0;
    mclk_out = 1'b0;
    cmd = 8'h00;
end

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
		M1 = 1'b1;
        RESET <= 1'b1;
        DATA <= 8'bZ;
        rdata_drive <= 1'b0;
		data_drive <= 1'b0;
    end else begin
        case (state)
            GET_CMD: begin
                cmd <= RDATA;
                case (cmd[3:0])
                    CMD_RESET: begin // Reset
                        RESET <= 1'b0;
                        state <= IDLE;
                    end
                    CMD_STATUS: begin // Get Status
                        state <= GET_STATUS;
                        rdata_drive <= 1'b1;
                    end
                    default: state <= GET_ADDR_L;
                endcase
            end

            GET_ADDR_L: begin
                ADDR[7:0] <= RDATA;
                state <= GET_ADDR_H;
            end

            GET_ADDR_H: begin
                ADDR[15:8] <= RDATA;
                if (cmd[0]) begin
                    state <= WRITE_DATA;
                    rdata_drive <= 1'b0;
                end else begin
                    state <= READ_MSXBUS;
                    DATA <= 8'hff;
                end
            end 

            WRITE_DATA: begin
                DATA <= RDATA;
                state <= WRITE_MSXBUS;
            end

            WRITE_MSXBUS: begin
                DATA <= RDATA;
                MREQ <= cmd[1];
                IORQ <= !cmd[1];
                RD <= 1'b1;
                WR <= 1'b0;
                if (!cmd[1] && !cmd[5]) begin  // If MREQ is active
                    SLTSL1 <= cmd[4];
                    SLTSL2 <= !cmd[4];
                end
                if (RELEASE)
                    state <= COMPLETE;
            end

            READ_MSXBUS: begin
                // Control signals setup
                DATA <= 8'hZ;
                MREQ <= cmd[1];
                IORQ <= !cmd[1];
                RD <= 1'b0;
                WR <= 1'b1;
                rdata_drive <= 1'b1;
                rdata_out <= DATA;
                if (!cmd[1] && !cmd[5]) begin  // If MREQ is active
                    SLTSL1 <= cmd[4];
                    SLTSL2 <= !cmd[4];
                end
                if (RELEASE)
                    state <= COMPLETE;
            end

            IDLE, COMPLETE: begin
                RD <= 1'b1;
                WR <= 1'b1;
                MREQ <= 1'b1;
                IORQ <= 1'b1;
                SLTSL1 <= 1'b1;
                SLTSL2 <= 1'b1;
                DATA <= 8'bZ;
                RESET <= 1'b1;
                rdata_drive <= 1'b0;
                state <= IDLE;
            end
        endcase
    end
end

// Remove the old combinational chip select logic block
endmodule

