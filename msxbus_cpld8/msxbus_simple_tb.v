`timescale 1ns/1ps

module msxbus_simple_tb;
    reg CS, CLK, PCLK;
    reg [1:0] SW;
    reg INT, BUSDIR, WAIT;
    wire [7:0] RDATA;
    wire [7:0] DATA;
    wire RD, WR, MREQ, IORQ, RESET;
    wire [15:0] ADDR;
    wire SLTSL1, SLTSL2;
    wire SLTSL1_CS1, SLTSL1_CS2, SLTSL1_CS12;
    wire SLTSL2_CS1, SLTSL2_CS2, SLTSL2_CS12;
    wire CS1, CS2, CS12;
    wire MCLK, SWOUT, RFSH, M1;

    reg [7:0] rdata_reg;
    reg [7:0] data_reg;
    
    assign RDATA = rdata_reg;
    assign DATA = data_reg;

    // Instantiate the msxbus_simple module
    msxbus_simple uut (
        .CS(CS),
        .CLK(CLK),
        .PCLK(PCLK),
        .RDATA(RDATA),
        .SW(SW),
        .INT(INT),
        .BUSDIR(BUSDIR),
        .WAIT(WAIT),
        .DATA(DATA),
        .RD(RD),
        .WR(WR),
        .MREQ(MREQ),
        .IORQ(IORQ),
        .RESET(RESET),
        .ADDR(ADDR),
        .SLTSL1(SLTSL1),
        .SLTSL2(SLTSL2),
        .SLTSL1_CS1(SLTSL1_CS1),
        .SLTSL1_CS2(SLTSL1_CS2),
        .SLTSL1_CS12(SLTSL1_CS12),
        .SLTSL2_CS1(SLTSL2_CS1),
        .SLTSL2_CS2(SLTSL2_CS2),
        .SLTSL2_CS12(SLTSL2_CS12),
        .CS1(CS1),
        .CS2(CS2),
        .CS12(CS12),
        .MCLK(MCLK),
        .SWOUT(SWOUT),
        .RFSH(RFSH),
        .M1(M1)
    );

    // Add simulation directives for Quartus
    initial begin
        $dumpfile("msxbus_simple_tb.vcd");
        $dumpvars(0, msxbus_simple_tb);
    end

    // Clock generation with more realistic timing (50MHz)
    initial begin
        CLK = 0;
        forever #10 CLK = ~CLK;  // 50MHz (20ns period)
    end

    initial begin
        PCLK = 0;
        forever #20 PCLK = ~PCLK;  // 25MHz
    end

    // Test stimulus with longer delays for stability
    initial begin
        // Initialize signals
        CS = 1;
        SW = 2'b00;
        INT = 0;
        BUSDIR = 0;
        WAIT = 1;
        rdata_reg = 8'hZZ;
        data_reg = 8'hAA;

        #1000;  // Longer initial delay

        // Start memory read transaction
        CS = 0;
        #100;
        
        // Send CMD_MEM_READ command (0x00)
        rdata_reg = 8'h00;
        #100;
        
        // Send address low byte (0x00)
        rdata_reg = 8'h00;
        #100;
        
        // Send address high byte (0x40)
        rdata_reg = 8'h40;
        #100;

        // Wait for memory read operation
        #200;

        // End transaction
        CS = 1;
        rdata_reg = 8'hZZ;

        // Wait for completion
        #1000;
        $stop;  // Use $stop instead of $finish for Quartus
    end

    // Enhanced monitoring
    initial begin
        $monitor("Time=%0t CS=%b ADDR=%h DATA=%h RD=%b MREQ=%b SLTSL1=%b CS1=%b",
                 $time, CS, ADDR, DATA, RD, MREQ, SLTSL1, CS1);
    end

endmodule