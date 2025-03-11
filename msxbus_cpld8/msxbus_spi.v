//-----------------------------------------------------------------------------
// MSX BUS Master Controller (MAX II CPLD)
//-----------------------------------------------------------------------------
//
// Description:
//   - Implementation of MSX Bus Master Controller
//   - Supports memory and I/O read/write operations
//   - Controls slot selection and chip select signals
//   - 1bit SPI slave interface (CS, CLK, MISO, MOSI)
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

module msxbus_spi (
    input CLK,          // System clock
    input SPI_CS,       // SPI chip select
    input SPI_CLK,      // SPI clock
    input SPI_MOSI,     // SPI data in
    output reg SPI_MISO,// SPI data out
    input WAIT,         // MSX WAIT signal
    output reg RD,      // MSX read signal
    output reg WR,      // MSX write signal
    output reg MREQ,    // MSX memory request
    output reg IORQ,    // MSX IO request
    output reg RESET,   // MSX reset signal
    output reg [15:0] ADDR, // MSX address bus
    output reg SLTSL1,  // Slot select 1
    output reg SLTSL2,  // Slot select 2
    output reg SLTSL1_CS1,
    output reg SLTSL1_CS2,
    output reg SLTSL1_CS12,
    output reg SLTSL2_CS1,
    output reg SLTSL2_CS2,
    output reg SLTSL2_CS12,
    output reg CS1,
    output reg CS2,
    output reg CS12,
    output reg M1,
    inout [7:0] DATA    // MSX data bus
);

    // Internal registers
    reg [7:0] cmd;
    reg [7:0] data_out;
    reg [7:0] spi_in_shift;
    reg [7:0] spi_out_shift;
    reg data_drive;
    reg [3:0] bit_count;
    reg [2:0] byte_count;
    reg [2:0] state;
    reg [1:0] delay_count;

    // State definitions
    localparam 
        IDLE = 3'd0,
        GET_CMD = 3'd1,
        GET_ADDR_L = 3'd2,
        GET_ADDR_H = 3'd3,
        SET_CONTROL = 3'd4,
        DELAY_STATE = 3'd5,
        WAIT_STATE = 3'd6,
        COMPLETE = 3'd7;

    // Bidirectional data bus control
    assign DATA = data_drive ? data_out : 8'bZ;

    // SPI receive logic
    always @(negedge SPI_CLK or posedge SPI_CS) begin
        if (SPI_CS) begin
            bit_count <= 4'd0;
            byte_count <= 3'd0;
            state <= IDLE;
        end else begin
            spi_in_shift <= {spi_in_shift[6:0], SPI_MOSI};
            bit_count <= bit_count + 1;

            if (bit_count == 4'd7) begin
                bit_count <= 4'd0;
                case (byte_count)
                    3'd0: begin
                        cmd <= spi_in_shift;
                        byte_count <= byte_count + 1;
                    end
                    3'd1: begin
                        ADDR[7:0] <= spi_in_shift;
                        byte_count <= byte_count + 1;
                    end
                    3'd2: begin
                        ADDR[15:8] <= spi_in_shift;
                        byte_count <= byte_count + 1;
                        if (cmd[3:0] != 4'h5 && cmd[3:0] != 4'h8) begin
                            state <= SET_CONTROL;
                        end
                    end
                    3'd3: begin
                        if (cmd[0]) begin // Write operation
                            data_out <= spi_in_shift;
                            data_drive <= 1'b1;
                        end
                        byte_count <= 3'd0;
                        state <= SET_CONTROL;
                    end
                endcase
            end
        end
    end

    // MSX bus control logic
    always @(posedge CLK or posedge SPI_CS) begin
        if (SPI_CS) begin
            RD <= 1'b1;
            WR <= 1'b1;
            MREQ <= 1'b1;
            IORQ <= 1'b1;
            RESET <= 1'b1;
            M1 <= 1'b1;
            SLTSL1 <= 1'b1;
            SLTSL2 <= 1'b1;
            data_drive <= 1'b0;
            state <= IDLE;
        end else begin
            case (state)
                SET_CONTROL: begin
                    MREQ <= cmd[1];
                    IORQ <= !cmd[1];
                    RD <= cmd[0];
                    WR <= !cmd[0];
                    M1 <= cmd[7];

                    if (!cmd[1]) begin // Memory access
                        SLTSL1 <= !cmd[4];
                        SLTSL2 <= cmd[4];
                        if (!cmd[4]) begin
                            SLTSL1_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                            SLTSL1_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                            SLTSL1_CS12 <= ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10)) ? 1'b0 : 1'b1;
                        end else begin
                            SLTSL2_CS1 <= (ADDR[15:14] == 2'b01) ? 1'b0 : 1'b1;
                            SLTSL2_CS2 <= (ADDR[15:14] == 2'b10) ? 1'b0 : 1'b1;
                            SLTSL2_CS12 <= ((ADDR[15:14] == 2'b01) || (ADDR[15:14] == 2'b10)) ? 1'b0 : 1'b1;
                        end
                    end
                    delay_count <= 2'b11;
                    state <= DELAY_STATE;
                end

                DELAY_STATE: begin
                    if (delay_count == 2'b00)
                        state <= WAIT_STATE;
                    else
                        delay_count <= delay_count - 1;
                end

                WAIT_STATE: begin
                    if (WAIT) begin
                        spi_out_shift <= DATA;
                        state <= COMPLETE;
                    end
                end

                COMPLETE: begin
                    RD <= 1'b1;
                    WR <= 1'b1;
                    MREQ <= 1'b1;
                    IORQ <= 1'b1;
                    SLTSL1 <= 1'b1;
                    SLTSL2 <= 1'b1;
                    data_drive <= 1'b0;
                    state <= IDLE;
                end
            endcase
        end
    end

    // SPI transmit logic
    always @(posedge SPI_CLK or posedge SPI_CS) begin
        if (SPI_CS)
            SPI_MISO <= 1'b0;
        else begin
            case (state)
                WAIT_STATE: SPI_MISO <= 1'b1;  // ACK
                COMPLETE: SPI_MISO <= spi_out_shift[7-bit_count];
                default: SPI_MISO <= 1'b0;
            endcase
        end
    end

endmodule