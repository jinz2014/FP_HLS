onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /TB/acc_i/clk
add wave -noupdate -radix hexadecimal /TB/acc_i/acc_in
add wave -noupdate -radix hexadecimal /TB/acc_i/acc_in_valid
add wave -noupdate -radix hexadecimal /TB/acc_i/reg_data_valid
add wave -noupdate -radix hexadecimal /TB/acc_i/sum_out_valid
add wave -noupdate -radix hexadecimal -subitemconfig {{/TB/acc_i/vec[5]} {-radix hexadecimal} {/TB/acc_i/vec[4]} {-radix hexadecimal} {/TB/acc_i/vec[3]} {-radix hexadecimal} {/TB/acc_i/vec[2]} {-radix hexadecimal} {/TB/acc_i/vec[1]} {-radix hexadecimal} {/TB/acc_i/vec[0]} {-radix hexadecimal}} /TB/acc_i/vec
add wave -noupdate -radix hexadecimal /TB/acc_i/acc_out
add wave -noupdate -radix hexadecimal /TB/acc_i/acc_out_valid
add wave -noupdate -radix hexadecimal /TB/acc_i/reg_data_in
add wave -noupdate -radix hexadecimal /TB/acc_i/mux0_out
add wave -noupdate -radix hexadecimal /TB/acc_i/mux1_out
add wave -noupdate -radix hexadecimal /TB/acc_i/mux2_out
add wave -noupdate -radix hexadecimal /TB/acc_i/sum_out
add wave -noupdate -radix hexadecimal /TB/acc_i/sel0
add wave -noupdate -radix hexadecimal /TB/acc_i/sel1
add wave -noupdate -radix hexadecimal /TB/acc_i/sel2
add wave -noupdate -radix hexadecimal /TB/acc_i/sum_in_valid
add wave -noupdate -radix hexadecimal /TB/acc_i/reg_data_valid_clr
add wave -noupdate -radix hexadecimal /TB/acc_i/reg_data_en
add wave -noupdate -radix hexadecimal /TB/acc_i/acc_rdy
add wave -noupdate -radix hexadecimal /TB/acc_i/reg_data
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {110 ns} 0}
configure wave -namecolwidth 150
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ns} {1003 ns}
