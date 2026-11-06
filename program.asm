; Multi-byte Addition Example ($0002:$0003 + $0004:$0005)
; Results are stored into $0006:$0007

MAIN:     LDA #$FF       ; Setup number 1: $01FF
          STA $0002      ; Low byte
          LDA #$01
          STA $0003      ; High byte

          LDA #$02       ; Setup number 2: $0002
          STA $0004      ; Low byte
          LDA #$00
          STA $0005      ; High byte

          JSR ADD16      ; Jump to Subroutine to add them together
          BRK            ; Program finish ($01FF + $0002 = $0201)

; --- 16-bit Addition Subroutine ---
ADD16:    CLC            ; Clear Carry flag before starting addition
          LDA $0002      ; Load low byte of number 1
          ADC $0004      ; Add low byte of number 2 (with carry)
          STA $0006      ; Store in low byte of result

          LDA $0003      ; Load high byte of number 1
          ADC $0005      ; Add high byte of number 2 (plus any carry from before)
          STA $0007      ; Store in high byte of result
          RTS            ; Return From Subroutine
