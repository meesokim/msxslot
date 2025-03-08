# Create work library
if [file exists work] {
    vdel -lib work -all
}
vlib work

# Compile design files
vcom -work work msxbus_simple.v
vcom -work work msxbus_simple_tb.v

# Start simulation
vsim -L altera_mf_ver -L lpm_ver -L sgate_ver -L altera_primitives_ver work.msxbus_simple_tb

# Add waves
add wave -position insertpoint sim:/msxbus_simple_tb/*

# Run simulation
run 3000ns