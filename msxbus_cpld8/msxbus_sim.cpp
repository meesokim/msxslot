
#include "Vmsxbus_simple.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    
    // Create VCD file
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    
    // Instance DUT
    Vmsxbus_simple* dut = new Vmsxbus_simple;
    dut->trace(tfp, 99);
    tfp->open("msxbus.vcd");
    
    // Initialize inputs
    dut->CS = 1;
    dut->CLK = 0;
    dut->PCLK = 1;
    dut->SW = 0;
    dut->INT = 0;
    dut->BUSDIR = 0;
    dut->WAIT = 1;
    
    // Simulation time
    int main_time = 0;
    int step = 0;
    // Main simulation loop
    for (main_time = 0; main_time < 1000; main_time += 10, step++)
    {
        dut->CLK = !dut->CLK;
        switch (step) {
            case 0:
                dut->CS = 0;
                dut->PCLK = 0;
                dut->RDATA = 0x00;
                break;
            case 1:
                dut->PCLK = 1;
                break;
            case 2:
                dut->PCLK = 0;
                dut->RDATA = 0x00;
                break;                
            case 3:
                dut->PCLK = 1;
                break;
            case 4:
                dut->PCLK = 0;
                dut->RDATA = 0x40;
                break;                
            case 5:
                dut->PCLK = 1;
                break;
            case 6:
                dut->PCLK = 0;
                dut->DATA = 0x41;
                break;                
            case 7:
                dut->PCLK = 1;
                dut->CS = 1;
                break;
            default:
                continue;
        }
        // Evaluate model
        dut->eval();
        tfp->dump(main_time);
    }
    // Cleanup
    tfp->close();
    delete dut;
    delete tfp;
    
    return 0;
}