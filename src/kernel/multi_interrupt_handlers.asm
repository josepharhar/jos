extern irq_1param
extern irq_2param
extern irq_syscall

global cr2_register
cr2_register:
  dq 0

irq_temp:
  dq 0

global irq_error_code
irq_error_code:
  dq 0


global irq0_handler
irq0_handler:
  push rdi
  mov rdi,0
  jmp irq_1param

global irq1_handler
irq1_handler:
  push rdi
  mov rdi,1
  jmp irq_1param

global irq2_handler
irq2_handler:
  push rdi
  mov rdi,2
  jmp irq_1param

global irq3_handler
irq3_handler:
  push rdi
  mov rdi,3
  jmp irq_1param

global irq4_handler
irq4_handler:
  push rdi
  mov rdi,4
  jmp irq_1param

global irq5_handler
irq5_handler:
  push rdi
  mov rdi,5
  jmp irq_1param

global irq6_handler
irq6_handler:
  push rdi
  mov rdi,6
  jmp irq_1param

global irq7_handler
irq7_handler:
  push rdi
  mov rdi,7
  jmp irq_1param

global irq8_handler
irq8_handler:
  push rsi
  mov rsi,[rsp+8]
  push rdi
  mov rdi,8
  jmp irq_2param

global irq9_handler
irq9_handler:
  push rdi
  mov rdi,9
  jmp irq_1param

global irq10_handler
irq10_handler:
  push rsi
  mov rsi,[rsp+8]
  push rdi
  mov rdi,10
  jmp irq_2param

global irq11_handler
irq11_handler:
  push rsi
  mov rsi,[rsp+8]
  push rdi
  mov rdi,11
  jmp irq_2param

global irq12_handler
irq12_handler:
  push rsi
  mov rsi,[rsp+8]
  push rdi
  mov rdi,12
  jmp irq_2param

global irq13_handler
irq13_handler:
  push rsi
  mov rsi,[rsp+8]
  push rdi
  mov rdi,13
  jmp irq_2param

global irq14_handler
irq14_handler:
  ; save rsi
  mov [irq_temp],rsi

  ; use rsi to save cr2
  mov rsi,cr2
  mov [cr2_register],rsi

  ; use rsi to save error code
  mov rsi,[rsp]
  mov [irq_error_code],rsi
  ; pop error code
  add rsp,8

  ; restore rsi
  mov rsi,[irq_temp]

  ; save rdi and use it for irq#
  push rdi
  mov rdi,14

  ; use 1param since we popped error code
  jmp irq_1param

;  push rsi
;  mov rsi,cr2
;  mov [cr2_register],rsi
;  mov rsi,[rsp+8]
;  push rdi
;  mov rdi,14
;  jmp irq_2param

global irq15_handler
irq15_handler:
  push rdi
  mov rdi,15
  jmp irq_1param

global irq16_handler
irq16_handler:
  push rdi
  mov rdi,16
  jmp irq_1param

global irq17_handler
irq17_handler:
  push rdi
  mov rdi,17
  jmp irq_1param

global irq18_handler
irq18_handler:
  push rdi
  mov rdi,18
  jmp irq_1param

global irq19_handler
irq19_handler:
  push rdi
  mov rdi,19
  jmp irq_1param

global irq20_handler
irq20_handler:
  push rdi
  mov rdi,20
  jmp irq_1param

global irq21_handler
irq21_handler:
  push rdi
  mov rdi,21
  jmp irq_1param

global irq22_handler
irq22_handler:
  push rdi
  mov rdi,22
  jmp irq_1param

global irq23_handler
irq23_handler:
  push rdi
  mov rdi,23
  jmp irq_1param

global irq24_handler
irq24_handler:
  push rdi
  mov rdi,24
  jmp irq_1param

global irq25_handler
irq25_handler:
  push rdi
  mov rdi,25
  jmp irq_1param

global irq26_handler
irq26_handler:
  push rdi
  mov rdi,26
  jmp irq_1param

global irq27_handler
irq27_handler:
  push rdi
  mov rdi,27
  jmp irq_1param

global irq28_handler
irq28_handler:
  push rdi
  mov rdi,28
  jmp irq_1param

global irq29_handler
irq29_handler:
  push rdi
  mov rdi,29
  jmp irq_1param

global irq30_handler
irq30_handler:
  push rdi
  mov rdi,30
  jmp irq_1param

global irq31_handler
irq31_handler:
  push rdi
  mov rdi,31
  jmp irq_1param

global irq32_handler
irq32_handler:
  push rdi
  mov rdi,32
  jmp irq_1param

global irq33_handler
irq33_handler:
  push rdi
  mov rdi,33
  jmp irq_1param

global irq34_handler
irq34_handler:
  push rdi
  mov rdi,34
  jmp irq_1param

global irq35_handler
irq35_handler:
  push rdi
  mov rdi,35
  jmp irq_1param

global irq36_handler
irq36_handler:
  push rdi
  mov rdi,36
  jmp irq_1param

global irq37_handler
irq37_handler:
  push rdi
  mov rdi,37
  jmp irq_1param

global irq38_handler
irq38_handler:
  push rdi
  mov rdi,38
  jmp irq_1param

global irq39_handler
irq39_handler:
  push rdi
  mov rdi,39
  jmp irq_1param

global irq40_handler
irq40_handler:
  push rdi
  mov rdi,40
  jmp irq_1param

global irq41_handler
irq41_handler:
  push rdi
  mov rdi,41
  jmp irq_1param

global irq42_handler
irq42_handler:
  push rdi
  mov rdi,42
  jmp irq_1param

global irq43_handler
irq43_handler:
  push rdi
  mov rdi,43
  jmp irq_1param

global irq44_handler
irq44_handler:
  push rdi
  mov rdi,44
  jmp irq_1param

global irq45_handler
irq45_handler:
  push rdi
  mov rdi,45
  jmp irq_1param

global irq46_handler
irq46_handler:
  push rdi
  mov rdi,46
  jmp irq_1param

global irq47_handler
irq47_handler:
  push rdi
  mov rdi,47
  jmp irq_1param

global irq48_handler
irq48_handler:
  push rdi
  mov rdi,48
  jmp irq_1param

global irq49_handler
irq49_handler:
  push rdi
  mov rdi,49
  jmp irq_1param

global irq50_handler
irq50_handler:
  push rdi
  mov rdi,50
  jmp irq_1param

global irq51_handler
irq51_handler:
  push rdi
  mov rdi,51
  jmp irq_1param

global irq52_handler
irq52_handler:
  push rdi
  mov rdi,52
  jmp irq_1param

global irq53_handler
irq53_handler:
  push rdi
  mov rdi,53
  jmp irq_1param

global irq54_handler
irq54_handler:
  push rdi
  mov rdi,54
  jmp irq_1param

global irq55_handler
irq55_handler:
  push rdi
  mov rdi,55
  jmp irq_1param

global irq56_handler
irq56_handler:
  push rdi
  mov rdi,56
  jmp irq_1param

global irq57_handler
irq57_handler:
  push rdi
  mov rdi,57
  jmp irq_1param

global irq58_handler
irq58_handler:
  push rdi
  mov rdi,58
  jmp irq_1param

global irq59_handler
irq59_handler:
  push rdi
  mov rdi,59
  jmp irq_1param

global irq60_handler
irq60_handler:
  push rdi
  mov rdi,60
  jmp irq_1param

global irq61_handler
irq61_handler:
  push rdi
  mov rdi,61
  jmp irq_1param

global irq62_handler
irq62_handler:
  push rdi
  mov rdi,62
  jmp irq_1param

global irq63_handler
irq63_handler:
  push rdi
  mov rdi,63
  jmp irq_1param

global irq64_handler
irq64_handler:
  push rdi
  mov rdi,64
  jmp irq_1param

global irq65_handler
irq65_handler:
  push rdi
  mov rdi,65
  jmp irq_1param

global irq66_handler
irq66_handler:
  push rdi
  mov rdi,66
  jmp irq_1param

global irq67_handler
irq67_handler:
  push rdi
  mov rdi,67
  jmp irq_1param

global irq68_handler
irq68_handler:
  push rdi
  mov rdi,68
  jmp irq_1param

global irq69_handler
irq69_handler:
  push rdi
  mov rdi,69
  jmp irq_1param

global irq70_handler
irq70_handler:
  push rdi
  mov rdi,70
  jmp irq_1param

global irq71_handler
irq71_handler:
  push rdi
  mov rdi,71
  jmp irq_1param

global irq72_handler
irq72_handler:
  push rdi
  mov rdi,72
  jmp irq_1param

global irq73_handler
irq73_handler:
  push rdi
  mov rdi,73
  jmp irq_1param

global irq74_handler
irq74_handler:
  push rdi
  mov rdi,74
  jmp irq_1param

global irq75_handler
irq75_handler:
  push rdi
  mov rdi,75
  jmp irq_1param

global irq76_handler
irq76_handler:
  push rdi
  mov rdi,76
  jmp irq_1param

global irq77_handler
irq77_handler:
  push rdi
  mov rdi,77
  jmp irq_1param

global irq78_handler
irq78_handler:
  push rdi
  mov rdi,78
  jmp irq_1param

global irq79_handler
irq79_handler:
  push rdi
  mov rdi,79
  jmp irq_1param

global irq80_handler
irq80_handler:
  push rdi
  mov rdi,80
  jmp irq_1param

global irq81_handler
irq81_handler:
  push rdi
  mov rdi,81
  jmp irq_1param

global irq82_handler
irq82_handler:
  push rdi
  mov rdi,82
  jmp irq_1param

global irq83_handler
irq83_handler:
  push rdi
  mov rdi,83
  jmp irq_1param

global irq84_handler
irq84_handler:
  push rdi
  mov rdi,84
  jmp irq_1param

global irq85_handler
irq85_handler:
  push rdi
  mov rdi,85
  jmp irq_1param

global irq86_handler
irq86_handler:
  push rdi
  mov rdi,86
  jmp irq_1param

global irq87_handler
irq87_handler:
  push rdi
  mov rdi,87
  jmp irq_1param

global irq88_handler
irq88_handler:
  push rdi
  mov rdi,88
  jmp irq_1param

global irq89_handler
irq89_handler:
  push rdi
  mov rdi,89
  jmp irq_1param

global irq90_handler
irq90_handler:
  push rdi
  mov rdi,90
  jmp irq_1param

global irq91_handler
irq91_handler:
  push rdi
  mov rdi,91
  jmp irq_1param

global irq92_handler
irq92_handler:
  push rdi
  mov rdi,92
  jmp irq_1param

global irq93_handler
irq93_handler:
  push rdi
  mov rdi,93
  jmp irq_1param

global irq94_handler
irq94_handler:
  push rdi
  mov rdi,94
  jmp irq_1param

global irq95_handler
irq95_handler:
  push rdi
  mov rdi,95
  jmp irq_1param

global irq96_handler
irq96_handler:
  push rdi
  mov rdi,96
  jmp irq_1param

global irq97_handler
irq97_handler:
  push rdi
  mov rdi,97
  jmp irq_1param

global irq98_handler
irq98_handler:
  push rdi
  mov rdi,98
  jmp irq_1param

global irq99_handler
irq99_handler:
  push rdi
  mov rdi,99
  jmp irq_1param

global irq100_handler
irq100_handler:
  push rdi
  mov rdi,100
  jmp irq_1param

global irq101_handler
irq101_handler:
  push rdi
  mov rdi,101
  jmp irq_1param

global irq102_handler
irq102_handler:
  push rdi
  mov rdi,102
  jmp irq_1param

global irq103_handler
irq103_handler:
  push rdi
  mov rdi,103
  jmp irq_1param

global irq104_handler
irq104_handler:
  push rdi
  mov rdi,104
  jmp irq_1param

global irq105_handler
irq105_handler:
  push rdi
  mov rdi,105
  jmp irq_1param

global irq106_handler
irq106_handler:
  push rdi
  mov rdi,106
  jmp irq_1param

global irq107_handler
irq107_handler:
  push rdi
  mov rdi,107
  jmp irq_1param

global irq108_handler
irq108_handler:
  push rdi
  mov rdi,108
  jmp irq_1param

global irq109_handler
irq109_handler:
  push rdi
  mov rdi,109
  jmp irq_1param

global irq110_handler
irq110_handler:
  push rdi
  mov rdi,110
  jmp irq_1param

global irq111_handler
irq111_handler:
  push rdi
  mov rdi,111
  jmp irq_1param

global irq112_handler
irq112_handler:
  push rdi
  mov rdi,112
  jmp irq_1param

global irq113_handler
irq113_handler:
  push rdi
  mov rdi,113
  jmp irq_1param

global irq114_handler
irq114_handler:
  push rdi
  mov rdi,114
  jmp irq_1param

global irq115_handler
irq115_handler:
  push rdi
  mov rdi,115
  jmp irq_1param

global irq116_handler
irq116_handler:
  push rdi
  mov rdi,116
  jmp irq_1param

global irq117_handler
irq117_handler:
  push rdi
  mov rdi,117
  jmp irq_1param

global irq118_handler
irq118_handler:
  push rdi
  mov rdi,118
  jmp irq_1param

global irq119_handler
irq119_handler:
  push rdi
  mov rdi,119
  jmp irq_1param

global irq120_handler
irq120_handler:
  push rdi
  mov rdi,120
  jmp irq_1param

global irq121_handler
irq121_handler:
  push rdi
  mov rdi,121
  jmp irq_1param

global irq122_handler
irq122_handler:
  push rdi
  mov rdi,122
  jmp irq_1param

global irq123_handler
irq123_handler:
  push rdi
  mov rdi,123
  jmp irq_1param

global irq124_handler
irq124_handler:
  push rdi
  mov rdi,124
  jmp irq_1param

global irq125_handler
irq125_handler:
  push rdi
  mov rdi,125
  jmp irq_1param

global irq126_handler
irq126_handler:
  push rdi
  mov rdi,126
  jmp irq_1param

global irq127_handler
irq127_handler:
  push rdi
  mov rdi,127
  jmp irq_1param

global irq128_handler
irq128_handler:
  push rdi
  mov rdi,128
  jmp irq_syscall

global irq129_handler
irq129_handler:
  push rdi
  mov rdi,129
  jmp irq_1param

global irq130_handler
irq130_handler:
  push rdi
  mov rdi,130
  jmp irq_1param

global irq131_handler
irq131_handler:
  push rdi
  mov rdi,131
  jmp irq_1param

global irq132_handler
irq132_handler:
  push rdi
  mov rdi,132
  jmp irq_1param

global irq133_handler
irq133_handler:
  push rdi
  mov rdi,133
  jmp irq_1param

global irq134_handler
irq134_handler:
  push rdi
  mov rdi,134
  jmp irq_1param

global irq135_handler
irq135_handler:
  push rdi
  mov rdi,135
  jmp irq_1param

global irq136_handler
irq136_handler:
  push rdi
  mov rdi,136
  jmp irq_1param

global irq137_handler
irq137_handler:
  push rdi
  mov rdi,137
  jmp irq_1param

global irq138_handler
irq138_handler:
  push rdi
  mov rdi,138
  jmp irq_1param

global irq139_handler
irq139_handler:
  push rdi
  mov rdi,139
  jmp irq_1param

global irq140_handler
irq140_handler:
  push rdi
  mov rdi,140
  jmp irq_1param

global irq141_handler
irq141_handler:
  push rdi
  mov rdi,141
  jmp irq_1param

global irq142_handler
irq142_handler:
  push rdi
  mov rdi,142
  jmp irq_1param

global irq143_handler
irq143_handler:
  push rdi
  mov rdi,143
  jmp irq_1param

global irq144_handler
irq144_handler:
  push rdi
  mov rdi,144
  jmp irq_1param

global irq145_handler
irq145_handler:
  push rdi
  mov rdi,145
  jmp irq_1param

global irq146_handler
irq146_handler:
  push rdi
  mov rdi,146
  jmp irq_1param

global irq147_handler
irq147_handler:
  push rdi
  mov rdi,147
  jmp irq_1param

global irq148_handler
irq148_handler:
  push rdi
  mov rdi,148
  jmp irq_1param

global irq149_handler
irq149_handler:
  push rdi
  mov rdi,149
  jmp irq_1param

global irq150_handler
irq150_handler:
  push rdi
  mov rdi,150
  jmp irq_1param

global irq151_handler
irq151_handler:
  push rdi
  mov rdi,151
  jmp irq_1param

global irq152_handler
irq152_handler:
  push rdi
  mov rdi,152
  jmp irq_1param

global irq153_handler
irq153_handler:
  push rdi
  mov rdi,153
  jmp irq_1param

global irq154_handler
irq154_handler:
  push rdi
  mov rdi,154
  jmp irq_1param

global irq155_handler
irq155_handler:
  push rdi
  mov rdi,155
  jmp irq_1param

global irq156_handler
irq156_handler:
  push rdi
  mov rdi,156
  jmp irq_1param

global irq157_handler
irq157_handler:
  push rdi
  mov rdi,157
  jmp irq_1param

global irq158_handler
irq158_handler:
  push rdi
  mov rdi,158
  jmp irq_1param

global irq159_handler
irq159_handler:
  push rdi
  mov rdi,159
  jmp irq_1param

global irq160_handler
irq160_handler:
  push rdi
  mov rdi,160
  jmp irq_1param

global irq161_handler
irq161_handler:
  push rdi
  mov rdi,161
  jmp irq_1param

global irq162_handler
irq162_handler:
  push rdi
  mov rdi,162
  jmp irq_1param

global irq163_handler
irq163_handler:
  push rdi
  mov rdi,163
  jmp irq_1param

global irq164_handler
irq164_handler:
  push rdi
  mov rdi,164
  jmp irq_1param

global irq165_handler
irq165_handler:
  push rdi
  mov rdi,165
  jmp irq_1param

global irq166_handler
irq166_handler:
  push rdi
  mov rdi,166
  jmp irq_1param

global irq167_handler
irq167_handler:
  push rdi
  mov rdi,167
  jmp irq_1param

global irq168_handler
irq168_handler:
  push rdi
  mov rdi,168
  jmp irq_1param

global irq169_handler
irq169_handler:
  push rdi
  mov rdi,169
  jmp irq_1param

global irq170_handler
irq170_handler:
  push rdi
  mov rdi,170
  jmp irq_1param

global irq171_handler
irq171_handler:
  push rdi
  mov rdi,171
  jmp irq_1param

global irq172_handler
irq172_handler:
  push rdi
  mov rdi,172
  jmp irq_1param

global irq173_handler
irq173_handler:
  push rdi
  mov rdi,173
  jmp irq_1param

global irq174_handler
irq174_handler:
  push rdi
  mov rdi,174
  jmp irq_1param

global irq175_handler
irq175_handler:
  push rdi
  mov rdi,175
  jmp irq_1param

global irq176_handler
irq176_handler:
  push rdi
  mov rdi,176
  jmp irq_1param

global irq177_handler
irq177_handler:
  push rdi
  mov rdi,177
  jmp irq_1param

global irq178_handler
irq178_handler:
  push rdi
  mov rdi,178
  jmp irq_1param

global irq179_handler
irq179_handler:
  push rdi
  mov rdi,179
  jmp irq_1param

global irq180_handler
irq180_handler:
  push rdi
  mov rdi,180
  jmp irq_1param

global irq181_handler
irq181_handler:
  push rdi
  mov rdi,181
  jmp irq_1param

global irq182_handler
irq182_handler:
  push rdi
  mov rdi,182
  jmp irq_1param

global irq183_handler
irq183_handler:
  push rdi
  mov rdi,183
  jmp irq_1param

global irq184_handler
irq184_handler:
  push rdi
  mov rdi,184
  jmp irq_1param

global irq185_handler
irq185_handler:
  push rdi
  mov rdi,185
  jmp irq_1param

global irq186_handler
irq186_handler:
  push rdi
  mov rdi,186
  jmp irq_1param

global irq187_handler
irq187_handler:
  push rdi
  mov rdi,187
  jmp irq_1param

global irq188_handler
irq188_handler:
  push rdi
  mov rdi,188
  jmp irq_1param

global irq189_handler
irq189_handler:
  push rdi
  mov rdi,189
  jmp irq_1param

global irq190_handler
irq190_handler:
  push rdi
  mov rdi,190
  jmp irq_1param

global irq191_handler
irq191_handler:
  push rdi
  mov rdi,191
  jmp irq_1param

global irq192_handler
irq192_handler:
  push rdi
  mov rdi,192
  jmp irq_1param

global irq193_handler
irq193_handler:
  push rdi
  mov rdi,193
  jmp irq_1param

global irq194_handler
irq194_handler:
  push rdi
  mov rdi,194
  jmp irq_1param

global irq195_handler
irq195_handler:
  push rdi
  mov rdi,195
  jmp irq_1param

global irq196_handler
irq196_handler:
  push rdi
  mov rdi,196
  jmp irq_1param

global irq197_handler
irq197_handler:
  push rdi
  mov rdi,197
  jmp irq_1param

global irq198_handler
irq198_handler:
  push rdi
  mov rdi,198
  jmp irq_1param

global irq199_handler
irq199_handler:
  push rdi
  mov rdi,199
  jmp irq_1param

global irq200_handler
irq200_handler:
  push rdi
  mov rdi,200
  jmp irq_1param

global irq201_handler
irq201_handler:
  push rdi
  mov rdi,201
  jmp irq_1param

global irq202_handler
irq202_handler:
  push rdi
  mov rdi,202
  jmp irq_1param

global irq203_handler
irq203_handler:
  push rdi
  mov rdi,203
  jmp irq_1param

global irq204_handler
irq204_handler:
  push rdi
  mov rdi,204
  jmp irq_1param

global irq205_handler
irq205_handler:
  push rdi
  mov rdi,205
  jmp irq_1param

global irq206_handler
irq206_handler:
  push rdi
  mov rdi,206
  jmp irq_1param

global irq207_handler
irq207_handler:
  push rdi
  mov rdi,207
  jmp irq_1param

global irq208_handler
irq208_handler:
  push rdi
  mov rdi,208
  jmp irq_1param

global irq209_handler
irq209_handler:
  push rdi
  mov rdi,209
  jmp irq_1param

global irq210_handler
irq210_handler:
  push rdi
  mov rdi,210
  jmp irq_1param

global irq211_handler
irq211_handler:
  push rdi
  mov rdi,211
  jmp irq_1param

global irq212_handler
irq212_handler:
  push rdi
  mov rdi,212
  jmp irq_1param

global irq213_handler
irq213_handler:
  push rdi
  mov rdi,213
  jmp irq_1param

global irq214_handler
irq214_handler:
  push rdi
  mov rdi,214
  jmp irq_1param

global irq215_handler
irq215_handler:
  push rdi
  mov rdi,215
  jmp irq_1param

global irq216_handler
irq216_handler:
  push rdi
  mov rdi,216
  jmp irq_1param

global irq217_handler
irq217_handler:
  push rdi
  mov rdi,217
  jmp irq_1param

global irq218_handler
irq218_handler:
  push rdi
  mov rdi,218
  jmp irq_1param

global irq219_handler
irq219_handler:
  push rdi
  mov rdi,219
  jmp irq_1param

global irq220_handler
irq220_handler:
  push rdi
  mov rdi,220
  jmp irq_1param

global irq221_handler
irq221_handler:
  push rdi
  mov rdi,221
  jmp irq_1param

global irq222_handler
irq222_handler:
  push rdi
  mov rdi,222
  jmp irq_1param

global irq223_handler
irq223_handler:
  push rdi
  mov rdi,223
  jmp irq_1param

global irq224_handler
irq224_handler:
  push rdi
  mov rdi,224
  jmp irq_1param

global irq225_handler
irq225_handler:
  push rdi
  mov rdi,225
  jmp irq_1param

global irq226_handler
irq226_handler:
  push rdi
  mov rdi,226
  jmp irq_1param

global irq227_handler
irq227_handler:
  push rdi
  mov rdi,227
  jmp irq_1param

global irq228_handler
irq228_handler:
  push rdi
  mov rdi,228
  jmp irq_1param

global irq229_handler
irq229_handler:
  push rdi
  mov rdi,229
  jmp irq_1param

global irq230_handler
irq230_handler:
  push rdi
  mov rdi,230
  jmp irq_1param

global irq231_handler
irq231_handler:
  push rdi
  mov rdi,231
  jmp irq_1param

global irq232_handler
irq232_handler:
  push rdi
  mov rdi,232
  jmp irq_1param

global irq233_handler
irq233_handler:
  push rdi
  mov rdi,233
  jmp irq_1param

global irq234_handler
irq234_handler:
  push rdi
  mov rdi,234
  jmp irq_1param

global irq235_handler
irq235_handler:
  push rdi
  mov rdi,235
  jmp irq_1param

global irq236_handler
irq236_handler:
  push rdi
  mov rdi,236
  jmp irq_1param

global irq237_handler
irq237_handler:
  push rdi
  mov rdi,237
  jmp irq_1param

global irq238_handler
irq238_handler:
  push rdi
  mov rdi,238
  jmp irq_1param

global irq239_handler
irq239_handler:
  push rdi
  mov rdi,239
  jmp irq_1param

global irq240_handler
irq240_handler:
  push rdi
  mov rdi,240
  jmp irq_1param

global irq241_handler
irq241_handler:
  push rdi
  mov rdi,241
  jmp irq_1param

global irq242_handler
irq242_handler:
  push rdi
  mov rdi,242
  jmp irq_1param

global irq243_handler
irq243_handler:
  push rdi
  mov rdi,243
  jmp irq_1param

global irq244_handler
irq244_handler:
  push rdi
  mov rdi,244
  jmp irq_1param

global irq245_handler
irq245_handler:
  push rdi
  mov rdi,245
  jmp irq_1param

global irq246_handler
irq246_handler:
  push rdi
  mov rdi,246
  jmp irq_1param

global irq247_handler
irq247_handler:
  push rdi
  mov rdi,247
  jmp irq_1param

global irq248_handler
irq248_handler:
  push rdi
  mov rdi,248
  jmp irq_1param

global irq249_handler
irq249_handler:
  push rdi
  mov rdi,249
  jmp irq_1param

global irq250_handler
irq250_handler:
  push rdi
  mov rdi,250
  jmp irq_1param

global irq251_handler
irq251_handler:
  push rdi
  mov rdi,251
  jmp irq_1param

global irq252_handler
irq252_handler:
  push rdi
  mov rdi,252
  jmp irq_1param

global irq253_handler
irq253_handler:
  push rdi
  mov rdi,253
  jmp irq_1param

global irq254_handler
irq254_handler:
  push rdi
  mov rdi,254
  jmp irq_1param

global irq255_handler
irq255_handler:
  push rdi
  mov rdi,255
  jmp irq_1param
