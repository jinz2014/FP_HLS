onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /TestMax/clk
add wave -noupdate -radix hexadecimal /TestMax/pipeEn
add wave -noupdate -radix hexadecimal /TestMax/a
add wave -noupdate -radix hexadecimal /TestMax/b
add wave -noupdate -radix hexadecimal /TestMax/result
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/clk
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/a
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/b
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/go
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/pipeEn
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/result
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/a_dly
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/b_dly
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/flt1_out
add wave -noupdate -radix hexadecimal /TestMax/fmaxi/rdy
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {159 ns} 0}
configure wave -namecolwidth 213
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
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
WaveRestoreZoom {0 ns} {339 ns}
