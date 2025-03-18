`timescale 1ns/1ps

module msxbus_simple_tb;
    // Signals declaration
    reg CS, CLK, PCLK;
    reg [1:0] SW;
    reg INT, BUSDIR, WAIT;
    wire [7:0] RDATA;
    wire [7:0] DATA;
    wire RD, WR, MREQ, IORQ, RESET, RWAIT;
    wire [15:0] ADDR;
    wire SLTSL1, SLTSL2;
    wire SLTSL1_CS1, SLTSL1_CS2, SLTSL1_CS12;
    wire SLTSL2_CS1, SLTSL2_CS2, SLTSL2_CS12;
    wire MCLK, SWOUT, RFSH, M1;

    // Bidirectional bus control
    reg [7:0] rdata_drive;
    reg [7:0] data_drive;
    assign RDATA = rdata_drive;
    assign DATA = data_drive;

    // Instance of module under test
    msxbus_simple uut (
        .CS(CS), .CLK(CLK), .PCLK(PCLK),
        .RDATA(RDATA), .SW(SW),
        .INT(INT), .BUSDIR(BUSDIR), .WAIT(WAIT),
        .DATA(DATA),
        .RD(RD), .WR(WR), .MREQ(MREQ), .IORQ(IORQ),
        .RESET(RESET), .RWAIT(RWAIT),
        .ADDR(ADDR),
        .SLTSL1(SLTSL1), .SLTSL2(SLTSL2),
        .SLTSL1_CS1(SLTSL1_CS1), .SLTSL1_CS2(SLTSL1_CS2),
        .SLTSL1_CS12(SLTSL1_CS12),
        .SLTSL2_CS1(SLTSL2_CS1), .SLTSL2_CS2(SLTSL2_CS2),
        .SLTSL2_CS12(SLTSL2_CS12),
        .MCLK(MCLK), .SWOUT(SWOUT), .RFSH(RFSH),
        .M1(M1)
    );

    // Clock generation
    initial begin
        CLK = 0;
        forever #10 CLK = ~CLK;  // 50MHz clock
    end

    initial begin
        PCLK = 0;
        forever #100 PCLK = ~PCLK;  // 5MHz clock
    end

    // Test stimulus
    initial begin
        // Initialize signals
        CS = 1;
        SW = 2'b00;
        INT = 0;
        BUSDIR = 0;
        WAIT = 1;
        rdata_drive = 8'hZZ;
        data_drive = 8'hZZ;

        // Wait for 100ns
        #100;

        // Test Memory Read
        CS = 0;
        rdata_drive = 8'h00;  // MEM_READ command
        #20;
        rdata_drive = 8'h34;  // Address Low
        #20;
        rdata_drive = 8'h40;  // Address High (0x4034)
        #20;
        data_drive = 8'hAA;   // Test data from MSX
        WAIT = 0;
        #40;
        WAIT = 1;
        #20;

        // Test Memory Write
        CS = 0;
        rdata_drive = 8'h01;  // MEM_WRITE command
        #20;
        rdata_drive = 8'hBB;  // Data to write
        #20;
        rdata_drive = 8'h56;  // Address Low
        #20;
        rdata_drive = 8'h80;  // Address High (0x8056)
        WAIT = 0;
        #40;
        WAIT = 1;
        #20;

        // Add more test cases as needed

        $finish;
    end

    // Monitor changes
    initial begin
        $monitor("Time=%0t CS=%b CMD=%h ADDR=%h DATA=%h RDATA=%h RD=%b WR=%b MREQ=%b IORQ=%b",
                 $time, CS, uut.CMD, ADDR, DATA, RDATA, RD, WR, MREQ, IORQ);
    end

endmodule