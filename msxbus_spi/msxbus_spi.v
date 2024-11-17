module msxbus (
    input  wire clk,             // System clock
    input  wire spi_cs,             // System reset
    input  wire spi_clk,         // SPI clock
    input  wire spi_mosi,        // SPI MOSI input
    output reg  spi_miso,        // SPI MISO output
    input  wire z80_wait,        // Z80 WAIT signal (active low)
    output reg  z80_rd,      // Z80 Memory Read signal (active low)
    output reg  z80_wr,      // Z80 Memory Write signal (active low)
    output reg  z80_mem,       // Z80 IO Read signal (active low)
    output reg  z80_io,       // Z80 IO Write signal (active low)
    output reg  z80_reset,
    output reg  msltsl1, msltsl2,
    output reg  [15:0] z80_addr, // Z80 Address bus
    inout  wire [7:0] z80_data   // Z80 Data bus
);

    // Internal signals
    reg [7:0] z80_data_out;      // Data to write to Z80
    reg z80_data_out_en;         // Data bus output enable
    reg [7:0] spi_data_in;       // Data received from SPI
    reg [7:0] spi_data_out;       // Data received from SPI
    reg [7:0] spi_cmd;           // SPI command
    reg [15:0] spi_addr;         // SPI address
    reg transaction_done;        // SPI transaction done flag
    reg [2:0] z80_cycle_state;   // State for Z80 clock cycle
    reg z80_clk;
    reg [3:0] clock;
    reg [6:0] spi_byte_cnt;
    reg [3:0] spi_bit_cnt;
    reg [7:0] spi_shift_reg;
    reg [7:0] spi_data;

    assign z80_data = z80_data_out_en ? z80_data_out : 8'bz;

    // Z80 clock cycle states
    parameter T0 = 3'd0;
    parameter T1 = 3'd1;
    parameter T2 = 3'd2;
    parameter T3 = 3'd3;
    parameter T4 = 3'd4;
    parameter DONE = 3'd5;

    reg [2:0] z80_state;

    always @(posedge clk) begin
        clock <= clock + 1;
        if (clock > 12) begin
            z80_clk <= !z80_clk;
            clock <= 0;
        end
    end
    // SPI command processing
    always @(posedge spi_clk or posedge spi_cs) begin
        if (spi_cs) begin
            spi_cmd <= 8'h00;
            spi_addr <= 16'h0000;
            transaction_done <= 1;
            spi_bit_cnt <= 0;
        end else begin
            // SPI protocol logic (similar to previous example)
            // Handles SPI command, address, and data reception
            // Shift in data from SPI MOSI
            spi_shift_reg <= {spi_shift_reg[6:0], spi_mosi};
            spi_bit_cnt <= spi_bit_cnt + 1;

            if (spi_bit_cnt == 7) begin
                // Full byte received
                case (spi_byte_cnt)
                    0: spi_cmd <= spi_shift_reg;           // First byte: Command
                    1: spi_addr[7:0] <= spi_shift_reg;     // Second byte: Address low byte
                    2: spi_addr[15:8] <= spi_shift_reg;    // Third byte: Address high byte
                    3: spi_data <= spi_shift_reg;          // Fourth byte: Data (for write commands)
                endcase
                spi_byte_cnt <= spi_byte_cnt + 1;
                spi_bit_cnt <= 0;

                // Reset byte counter when 4 bytes are received
                if (spi_byte_cnt == 3) begin
                    transaction_done <= 0;  // Start processing
                    spi_byte_cnt <= 0;
                end
            end            
        end
    end

    // Z80 bus control state machine
    always @(posedge z80_clk or negedge z80_reset) begin
        if (!z80_reset) begin
            z80_state <= T1;
            z80_mem <= 1;
            z80_io <= 1;
            z80_rd <= 1;
            z80_wr <= 1;
            msltsl1 <= 1;
            msltsl2 <= 1;
            z80_data_out_en <= 0;
        end else begin
            case (z80_state)
                T0: begin
                    if (spi_cmd > 0) begin
                        z80_state <= T1;
                    end
                end
                T1: begin
                    // Place address on bus
                    z80_addr <= spi_addr;

                    if (spi_cmd == 8'h01) begin
                        z80_mem <= 0;
                        z80_rd <= 0; // Memory Read
                        msltsl1 <= 0;
                    end else if (spi_cmd == 8'h02) begin       // Memory Write
                        z80_mem <= 0;
                        z80_wr <= 0;
                        z80_data_out <= spi_data;
                        z80_data_out_en <= 1;
                        msltsl1 <= 0;
                    end else if (spi_cmd == 8'h03) begin 
                        z80_io <= 0;
                        z80_rd <= 0; // IO Read
                    end else if (spi_cmd == 8'h04) begin               // IO Write
                        z80_io <= 0;
                        z80_wr <= 0;
                        z80_data_out <= spi_data;
                        z80_data_out_en <= 1;
                    end

                    z80_state <= T2;
                end

                T2: begin
                    // WAIT signal handling (Low Active)
                    if (z80_wait == 1) begin
                        z80_state <= T3;
                    end
                end

                T3: begin
                    if (spi_cmd == 8'h01) begin // Memory Read
                        spi_data_out <= z80_data;
                    end else if (spi_cmd == 8'h02) begin // Memory Write
                        z80_data_out_en <= 0;
                    end else if (spi_cmd == 8'h03) begin // IO Read
                        spi_data_out <= z80_data;
                    end else if (spi_cmd == 8'h04) begin // IO Write
                        z80_wr <= 1; // Deactivate IO WR
                        z80_data_out_en <= 0;
                    end
                    z80_state <= DONE;
                end

                DONE: begin
                    // End of cycle
                    z80_mem <= 1;
                    z80_io <= 1;
                    z80_wr <= 1;
                    z80_rd <= 1;
                    msltsl1 <= 1;
                    z80_state <= T0;
                end
            endcase
        end
    end

    // SPI MISO response logic
    always @(posedge spi_clk or posedge spi_cs) begin
        if (spi_cs) begin
            spi_miso <= 1'b0;
        end else begin
            if (spi_bit_cnt == 0)
            begin
                if (spi_byte_cnt == 0)
                    spi_data_in <= transaction_done ? 8'hFF : 8'h00;
                else if (spi_byte_cnt == 1)
                    spi_data_in <= spi_data_out;
                else 
                    spi_data_in <= 8'h00;
            end
            spi_miso <= spi_data_in[7-spi_bit_cnt];
        end
    end
endmodule
