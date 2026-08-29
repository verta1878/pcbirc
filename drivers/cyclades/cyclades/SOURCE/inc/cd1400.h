/* ====================================================================
 * cd1400.h — Cirrus Logic CD1400 UART Register Definitions
 * ====================================================================
 * Derived from Linux kernel include/linux/cyclades.h (GPL v2).
 * Original by Randolph Bentson and Ivan Passos (Cyclades Corp).
 *
 * The CD1400 is a 4-port UART used on Cyclades Cyclom-Y multiport
 * serial cards. It is memory-mapped, NOT I/O port mapped.
 * Registers are at (offset * 2) from chip base within the shared
 * memory window. Each chip occupies CyRegSize (0x400) bytes.
 * ====================================================================
 */

#ifndef CD1400_H
#define CD1400_H

/* -------------------------------------------------------------------- */
/* Card Geometry                                                        */
/* -------------------------------------------------------------------- */

#define CY_MAX_CHIPS        8       /* Max chips per card               */
#define CY_PORTS_PER_CHIP   4       /* Ports per CD1400                 */
#define CY_MAX_CHAR_FIFO    12      /* FIFO depth per channel           */
#define CD1400_MAX_SPEED    115200  /* Max baud rate                    */

/* Memory window sizes */
#define CY_ISA_WINDOW       0x2000  /* ISA: 8K window                   */
#define CY_PCI_WINDOW       0x4000  /* PCI: 16K window                  */

/* Card control offsets */
#define CY_REG_SIZE         0x0400  /* Register space per chip          */
#define CY_HW_RESET         0x1400  /* Hardware reset offset            */
#define CY_CLR_INTR         0x1800  /* Clear interrupt offset           */
#define CY_EPLD_REV         0x1E00  /* EPLD revision offset             */

/* CD1400 revision IDs */
#define CD1400_REV_G        0x46    /* 25 MHz clock                     */
#define CD1400_REV_J        0x48    /* 60 MHz clock                     */

/* Default shared memory base (ISA) */
#define CY_DEFAULT_MEMBASE  0xD4000

/* -------------------------------------------------------------------- */
/* Global Registers (per chip, not per channel)                         */
/* -------------------------------------------------------------------- */

#define CyGFRCR     (0x40*2)    /* Global Firmware Revision Code Reg    */
#define CyCAR       (0x68*2)    /* Channel Access Register              */
#define CyGCR       (0x4B*2)    /* Global Configuration Register        */
#define CySVRR      (0x67*2)    /* Service Request Register             */
#define CyRICR      (0x44*2)    /* Receive Interrupt Channel Register   */
#define CyTICR      (0x45*2)    /* Transmit Interrupt Channel Register  */
#define CyMICR      (0x46*2)    /* Modem Interrupt Channel Register     */
#define CyRIR       (0x6B*2)    /* Receive Interrupt Register           */
#define CyTIR       (0x6A*2)    /* Transmit Interrupt Register          */
#define CyMIR       (0x69*2)    /* Modem Interrupt Register             */
#define CyPPR       (0x7E*2)    /* Prescaler Period Register            */

/* CAR channel selection */
#define CyCHAN_0    0x00
#define CyCHAN_1    0x01
#define CyCHAN_2    0x02
#define CyCHAN_3    0x03

/* GCR modes */
#define CyCH0_SERIAL    0x00
#define CyCH0_PARALLEL  0x80

/* SVRR bits */
#define CySRModem       0x04
#define CySRTransmit    0x02
#define CySRReceive     0x01

/* IR busy/direction bits */
#define CyIRDirEq       0x80
#define CyIRBusy        0x40
#define CyIRUnfair      0x20
#define CyIRContext     0x1C
#define CyIRChannel     0x03

/* PPR clock values (prescaler for 1ms tick) */
#define CyCLOCK_20_1MS  0x27
#define CyCLOCK_25_1MS  0x31
#define CyCLOCK_25_5MS  0xF4
#define CyCLOCK_60_1MS  0x75
#define CyCLOCK_60_2MS  0xEA

/* -------------------------------------------------------------------- */
/* Virtual Registers (interrupt vectors)                                */
/* -------------------------------------------------------------------- */

#define CyRIVR      (0x43*2)    /* Receive Interrupt Vector Register    */
#define CyTIVR      (0x42*2)    /* Transmit Interrupt Vector Register   */
#define CyMIVR      (0x41*2)    /* Modem Interrupt Vector Register      */

#define CyIVRMask   0x07
#define CyIVRRxEx   0x07        /* Receive exception                    */
#define CyIVRRxOK   0x03        /* Receive OK                           */
#define CyIVRTxOK   0x02        /* Transmit OK                          */
#define CyIVRMdmOK  0x01        /* Modem OK                             */

/* -------------------------------------------------------------------- */
/* Data Registers                                                       */
/* -------------------------------------------------------------------- */

#define CyTDR       (0x63*2)    /* Transmit Data Register               */
#define CyRDSR      (0x62*2)    /* Receive Data/Status Register         */

/* RDSR status bits */
#define CyTIMEOUT   0x80        /* Receive timeout                      */
#define CySPECHAR   0x70        /* Special character detected           */
#define CyBREAK     0x08        /* Break received                       */
#define CyPARITY    0x04        /* Parity error                         */
#define CyFRAME     0x02        /* Framing error                        */
#define CyOVERRUN   0x01        /* Overrun error                        */

/* End of service */
#define CyEOSRR     (0x60*2)    /* End Of Service Request Register      */
#define CyMISR      (0x4C*2)    /* Modem Interrupt Status Register      */

/* -------------------------------------------------------------------- */
/* Channel Registers (selected via CAR)                                 */
/* -------------------------------------------------------------------- */

#define CyLIVR      (0x18*2)    /* Local Interrupt Vector Register      */
#define CyCCR       (0x05*2)    /* Channel Command Register             */

/* CCR commands — Format 1 */
#define CyCHAN_RESET        0x80
#define CyCHIP_RESET        0x81
#define CyFlushTransFIFO    0x82

/* CCR commands — Format 2 */
#define CyCOR_CHANGE        0x40
#define CyCOR1ch            0x02
#define CyCOR2ch            0x04
#define CyCOR3ch            0x08

/* CCR commands — Format 3 (send special chars) */
#define CySEND_SPEC_1       0x21
#define CySEND_SPEC_2       0x22
#define CySEND_SPEC_3       0x23
#define CySEND_SPEC_4       0x24

/* CCR commands — Format 4 (channel control) */
#define CyCHAN_CTL          0x10
#define CyDIS_RCVR          0x01
#define CyENB_RCVR          0x02
#define CyDIS_XMTR          0x04
#define CyENB_XMTR          0x08

/* Service Request Enable Register */
#define CySRER      (0x06*2)
#define CyMdmCh     0x80        /* Modem change                         */
#define CyRxData    0x10        /* Receive data                         */
#define CyTxRdy     0x04        /* Transmitter ready                    */
#define CyTxMpty    0x02        /* Transmitter empty                    */
#define CyNNDT      0x01        /* No new data timeout                  */

/* Channel Option Registers */
#define CyCOR1      (0x08*2)    /* Channel Option Register 1            */

/* COR1 parity */
#define CyPARITY_NONE   0x00
#define CyPARITY_0      0x20    /* Force 0 parity                       */
#define CyPARITY_1      0xA0    /* Force 1 parity                       */
#define CyPARITY_E      0x40    /* Even parity                          */
#define CyPARITY_O      0xC0    /* Odd parity                           */

/* COR1 stop bits */
#define Cy_1_STOP       0x00
#define Cy_1_5_STOP     0x04
#define Cy_2_STOP       0x08

/* COR1 data bits */
#define Cy_5_BITS       0x00
#define Cy_6_BITS       0x01
#define Cy_7_BITS       0x02
#define Cy_8_BITS       0x03

#define CyCOR2      (0x09*2)    /* Channel Option Register 2            */
#define CyIXM       0x80        /* Implied XON mode                     */
#define CyTxIBE     0x40        /* Tx in-band flow control enable       */
#define CyETC       0x20        /* Embedded Tx command enable           */
#define CyAUTO_TXFL 0x60        /* Auto Tx flow control                 */
#define CyLLM       0x10        /* Local loopback mode                  */
#define CyRLM       0x08        /* Remote loopback mode                 */
#define CyRtsAO     0x04        /* RTS automatic output                 */
#define CyCtsAE     0x02        /* CTS automatic enable                 */
#define CyDsrAE     0x01        /* DSR automatic enable                 */

#define CyCOR3      (0x0A*2)    /* Channel Option Register 3            */
#define CySPL_CH_DRANGE 0x80    /* Special char detect range            */
#define CySPL_CH_DET1   0x40    /* Special char detect on SCHR4-SCHR3   */
#define CyFL_CTRL_TRNSP 0x20   /* Flow control transparency            */
#define CySPL_CH_DET2   0x10    /* Special char detect on SCHR2-SCHR1   */
#define CyREC_FIFO      0x0F   /* Receive FIFO threshold mask          */

#define CyCOR4      (0x1E*2)    /* Channel Option Register 4            */
#define CyCOR5      (0x1F*2)    /* Channel Option Register 5            */

/* Channel Control Status Register */
#define CyCCSR      (0x0B*2)
#define CyRxEN      0x80
#define CyRxFloff   0x40
#define CyRxFlon    0x20
#define CyTxEN      0x08
#define CyTxFloff   0x04
#define CyTxFlon    0x02

/* Receive Data Count Register */
#define CyRDCR      (0x0E*2)

/* Special Character Registers */
#define CySCHR1     (0x1A*2)
#define CySCHR2     (0x1B*2)
#define CySCHR3     (0x1C*2)
#define CySCHR4     (0x1D*2)

/* Special Character Range */
#define CySCRL      (0x22*2)
#define CySCRH      (0x23*2)

/* Line Control */
#define CyLNC       (0x24*2)

/* Modem Change Option Registers */
#define CyMCOR1     (0x15*2)
#define CyMCOR2     (0x16*2)

/* Receive Timeout Period Register */
#define CyRTPR      (0x21*2)

/* Modem Signal Value Registers */
#define CyMSVR1     (0x6C*2)    /* Modem Signal Value Register 1        */
#define CyMSVR2     (0x6D*2)    /* Modem Signal Value Register 2        */

/* Modem signal bits (same for MSVR1, MSVR2, MCOR1, MCOR2) */
#define CyANY_DELTA 0xF0
#define CyDSR       0x80
#define CyCTS       0x40
#define CyRI        0x20
#define CyDCD       0x10
#define CyDTR       0x02
#define CyRTS       0x01

/* Prescaler Value Status Register */
#define CyPVSR      (0x6F*2)

/* Baud Rate Registers */
#define CyRBPR      (0x78*2)    /* Receive Baud Prescaler               */
#define CyRCOR      (0x7C*2)    /* Receive Clock Option Register        */
#define CyTBPR      (0x72*2)    /* Transmit Baud Prescaler              */
#define CyTCOR      (0x76*2)    /* Transmit Clock Option Register       */

/* -------------------------------------------------------------------- */
/* Baud Rate Tables                                                     */
/* -------------------------------------------------------------------- */

/* Clock option values (bits 2:0 of xCOR):
 *   000 = divide by 8
 *   001 = divide by 32
 *   010 = divide by 128
 *   011 = divide by 512
 *   100 = divide by 2048
 *
 * Baud = clock / (BPR * prescaler_divisor)
 * where prescaler_divisor is selected by xCOR bits
 */

/* Standard baud rate indices */
#define CY_BAUD_50      0
#define CY_BAUD_75      1
#define CY_BAUD_110     2
#define CY_BAUD_134     3
#define CY_BAUD_150     4
#define CY_BAUD_200     5
#define CY_BAUD_300     6
#define CY_BAUD_600     7
#define CY_BAUD_1200    8
#define CY_BAUD_1800    9
#define CY_BAUD_2400    10
#define CY_BAUD_4800    11
#define CY_BAUD_9600    12
#define CY_BAUD_19200   13
#define CY_BAUD_38400   14
#define CY_BAUD_57600   15
#define CY_BAUD_76800   16
#define CY_BAUD_115200  17
#define CY_BAUD_150000  18
#define CY_BAUD_230400  19

#endif /* CD1400_H */
