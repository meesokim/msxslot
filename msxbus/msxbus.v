`timescale 10ns/1ns

module msxbus (msltsl1, msltsl2, mcs1, mcs2, mcs12, mdata, maddr, mrd, mwr, miorq, mmreq, mreset, mm1, msxclk, 
					cs, a0, mode, ready, clk, md, 
					rw, mmeio, sltsl, mswsrc,
);

input clk, cs, a0, mode, rw, mmeio, sltsl;
inout reg [15:0] md;
output reg ready;
output reg msltsl1, msltsl2, mrd, mwr, miorq, mmreq, mreset, mm1;
output wire mcs1, mcs2, mcs12, mswsrc;
output reg [15:0] maddr;
inout reg [7:0] mdata;

output reg msxclk;
reg [15:0] addr;
reg [2:0] count;
reg [2:0] rcnt;
reg [2:0] wcnt;

assign mcs1   = (!msltsl1 | !msltsl2) & maddr[14] & !maddr[15] ? 1'b0 : 1'b1;
assign mcs2   = (!msltsl1 | !msltsl2) & !maddr[14] & maddr[15] ? 1'b0 : 1'b1;
assign mcs12  = (!msltsl1 | !msltsl2) & (!mcs1 | !mcs2) ? 1'b0 : 1'b1;
assign mswsrc = 1'b1;

initial begin
	count <= 0;
	ready <= 0;
	mreset <= 1;
	md[15:0] <= 16'bZ;
end

always @ (negedge clk) begin
	if (!cs)
		begin
			case (a0)
			0: begin
					if (!ready)
					begin
					md <= 16'bZ;
					maddr <= md;
					if (!rw)
						begin
							mmreq <= mmeio;
							miorq <= !mmeio;
							mrd <= rw;
							mwr <= !rw;
							msltsl1 <= sltsl;
							msltsl2 <= !sltsl;
							mdata <= 8'bZ;
						end
					else
						wcnt <= 4;
					end
				end
			1: begin
					if (!rw)
						begin
							mdata <= 8'bZ;
							md[7:0] <= mdata[7:0];
						end
					else 
						begin
						if (wcnt != 0)
							begin
								mdata[7:0] = md[15:8];
								wcnt = wcnt - 1;
								mmreq = mmeio;
								miorq = !mmeio;
								mrd = rw;
								mwr = !rw;
								msltsl1 = sltsl;
								msltsl2 = !sltsl;								
							end
						else
							begin
								msltsl1 <= 1'b1;
								msltsl2 <= 1'b1;
								mrd <= 1'b1;
								mwr <= 1'b1;
								mmreq <= 1'b1;
								miorq <= 1'b1;
							end
						end
				end
			endcase
		end
	else
		begin
		if (mode)
			begin
				msltsl1 <= 1'b1;
				msltsl2 <= 1'b1;
				mrd <= 1'b1;
				mwr <= 1'b1;
				mmreq <= 1'b1;
				miorq <= 1'b1;
				mdata <= 2'hff;
			end
		end
end

always @ (negedge clk) begin
	count <= count + 1;
	if (count == 0)
		msxclk = ~msxclk;
end

always @ (negedge clk) begin
	if (!cs)
		rcnt <= rcnt - 1;
	else
		begin
		ready <= 0;
		rcnt <= 7;
		end
	if (rcnt == 0)
		ready <= 1;
end

endmodule