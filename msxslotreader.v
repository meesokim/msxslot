/**************************************************************************************************
* SPI_to_MSX Converter 
* March 2016
**************************************************************************************************/
`timescale 10ns/1ns

module msxslotreader(sclk, cs, mosi, miso, address, data_bus, rd, wr, iorq, merq, cs1, cs2, cs12, sltsl, m1, reset, rfsh, msx_clk,
                  nwait, busdir, interrupt, sw1, sw2, swout, clk) ;

input mosi;
input cs;
input sclk;
inout wire [7:0] data_bus;
input clk;

output reg miso;
output wire [15:0] address;
output wire rd;      // control[0]
output wire wr;      // control[1]
output wire iorq;    // control[2]
output wire merq;    // control[3]
output wire sltsl;   // control[4]
output wire reset;   // control[5]
output wire m1;      // control[6]
output wire rfsh;    // control[7]
output wire cs1;     
output wire cs2;  
output wire cs12;

output reg msx_clk;
output wire swout;

input nwait;
input busdir;
input interrupt;
input sw1;
input sw2;

reg [31:0] control;
reg [7:0] data_in;
reg [7:0] data_out;
reg [4:0] count;
wire [7:0] status_reg;
reg [7:0] command_reg;
reg ready;
reg [2:0] msxcount;
reg [15:0] address_in;
reg reset_count;

//assign data_bus = status_reg[0] ? 

assign data_bus = ~wr ? control[7:0] : 8'bZ;
assign cs1 = ~sltsl & address[14] & ~address[15] ? 1'b0 : 1'b1;
assign cs2 = ~sltsl & ~address[14] & address[15] ? 1'b0 : 1'b1;
assign cs12 = ~sltsl & (~cs1 | ~cs2) ? 1'b0 : 1'b0;
assign rd    = ready ? ~control[31] : rd;
assign wr    = ready ? ~control[30] : wr;
assign iorq  = ready ? ~control[29] : iorq;
assign merq  = ready ? ~control[28] : merq;
assign sltsl = ready ? ~control[27] : sltsl;
assign reset = ready ? ~control[26] : reset;
assign m1    = ready ? ~control[25] : m1;
assign rfsh  = ready ? ~control[24] : rfsh;
assign swout = 1'b1;
assign status_reg[0] = nwait;
assign status_reg[1] = interrupt;
assign status_reg[2] = busdir;
assign status_reg[3] = sw1;
assign status_reg[4] = sw2;
assign address = ready ? control[23:8] : address;

initial
begin
    
address_in  = 16'h00;
data_in	   	= 8'h00;
data_out    = 8'h00;
control     = 32'h00;
count       = 5'b00000;
miso        = 1'bz;
msxcount    = 3'b000;
ready       = 1'b0;
reset_count = 1'b0;

end

always @ (negedge sclk or posedge cs)
	 if (cs)
    begin
	 if (count < 16)
		control = 32'b0;	
  	 count <= 0;
    ready <= 1'b1;
 	 end else
    begin
    ready <= 0;	 
    if (count == 0)
		data_in <= data_bus;
	 if (count < 32)
	   control[31-count] = mosi; 	
    count <= count + 5'b00001;
	 end

always @ (negedge clk) 
    begin
    msxcount <= msxcount + 3'b001;
	 if (msxcount == 3'b000)
		msx_clk = ~msx_clk;
    end
	 	
always @ (posedge sclk or negedge cs) 
   if (~cs)
	begin
   miso <= 1'bz;
   end else
	begin
	if(count < 8)
   miso <= status_reg[5'b00111-count];
   else if(count < 16)
   miso <= data_in[5'b01111-count];
	end
	
endmodule