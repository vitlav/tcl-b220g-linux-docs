# Методы стандартного опроса с вызовами board-ID

## DSDT.dsl:1780

```asl
Method (_SUB, 0, NotSerialized)  // _SUB: Subsystem ID
            {
                If ((\_SB.PSUB == "IDP07180"))
                {
                    Return ("IDP07180")
                }
                ElseIf ((\_SB.PSUB == "CLS07180"))
                {
                    If ((_BID () == Zero))
                    {
                        Return ("CLS07180")
                    }
                    Else
                    {
                        Return ("CLS17180")
                    }
                }
                ElseIf ((\_SB.PSUB == "IDPS7180"))
                {
                    Return ("IDPS7180")
                }
            }
```

## DSDT.dsl:34146

```asl
Method (_SUB, 0, NotSerialized)  // _SUB: Subsystem ID
                    {
                        If ((BSID () == Zero))
                        {
                            Return ("27822202")
                        }
                        Else
                        {
                            Return ("27822202")
                        }
                    }
```

## DSDT.dsl:34158

```asl
Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
                    {
                        If ((BSID () == Zero))
                        {
                            Name (RBUF, ResourceTemplate ()
                            {
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x003A
                                    }
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x002E
                                    }
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x002F
                                    }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000148,
                                }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000149,
                                }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000147,
                                }
                                GpioInt (Edge, ActiveHigh, ExclusiveAndWake, PullDown, 0x0000,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x0100
                                    }
                            })
                            Return (RBUF) /* \_SB_.ADSP.ADCM.AUDD._CRS.RBUF */
                        }
                        Else
                        {
                            Name (RBUC, ResourceTemplate ()
                            {
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x003A
                                    }
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x002E
                                    }
                                GpioIo (Exclusive, PullNone, 0x0000, 0x0640, IoRestrictionNone,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x002F
                                    }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000148,
                                }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000149,
                                }
                                Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                                {
                                    0x00000147,
                                }
                                GpioInt (Edge, ActiveHigh, ExclusiveAndWake, PullDown, 0x0000,
                                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                    )
                                    {   // Pin list
                                        0x0100
                                    }
                            })
                            Return (RBUC) /* \_SB_.ADSP.ADCM.AUDD._CRS.RBUC */
                        }
                    }
```

## DSDT.dsl:34261

```asl
Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
                        {
                            If ((BSID () == Zero))
                            {
                                Name (RBUF, ResourceTemplate ()
                                {
                                    GpioIo (Exclusive, PullDown, 0x0000, 0x0000, IoRestrictionNone,
                                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                        )
                                        {   // Pin list
                                            0x0048
                                        }
                                })
                                Return (RBUF) /* \_SB_.ADSP.ADCM.AUDD.MBHC._CRS.RBUF */
                            }
                            Else
                            {
                                Name (RBUC, ResourceTemplate ()
                                {
                                    GpioIo (Exclusive, PullDown, 0x0000, 0x0000, IoRestrictionNone,
                                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                                        )
                                        {   // Pin list
                                            0x0048
                                        }
                                })
                                Return (RBUC) /* \_SB_.ADSP.ADCM.AUDD.MBHC._CRS.RBUC */
                            }
                        }
```

## DSDT.dsl:34438

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    If (((\_SB.PLST == One) && ((BSID () == 0x02) || (BSID () == 0x03))))
                    {
                        Return (Zero)
                    }
                    Else
                    {
                        Return (0x0F)
                    }
                }
```

## DSDT.dsl:34617

```asl
Method (_SUB, 0, NotSerialized)  // _SUB: Subsystem ID
            {
                If (((\_SB.SOID == 0x01EF) || ((BREV () == One) && (BSID () == 0x02))))
                {
                    Return ("CLSA7180")
                }

                Return (\_SB.PSUB)
            }
```

## DSDT.dsl:34630

```asl
Method (_SUB, 0, NotSerialized)  // _SUB: Subsystem ID
            {
                If ((\_SB.SOID == 0x01EF))
                {
                    Return ("CLSA7180")
                }
                Else
                {
                    If ((BREV () == Zero))
                    {
                        If ((BSID () == Zero))
                        {
                            Return ("CLS07180")
                        }
                        Else
                        {
                            Return ("CLSQ7180")
                        }
                    }

                    If ((BREV () == One))
                    {
                        If ((BSID () == Zero))
                        {
                            Return ("CLSA7180")
                        }
                        Else
                        {
                            Return ("CLAQ7180")
                        }
                    }
                }
            }
```

## DSDT.dsl:34672

```asl
Method (_SUB, 0, NotSerialized)  // _SUB: Subsystem ID
            {
                If ((BSID () == Zero))
                {
                    Return (\_SB.PSUB)
                }
                Else
                {
                    Return ("27822202")
                }
            }
```

## DSDT.dsl:35080

```asl
Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (ABUF, ResourceTemplate ()
                {
                    Memory32Fixed (ReadWrite,
                        0x0AE00000,         // Address Base
                        0x00151000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x088E0000,         // Address Base
                        0x000F4000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x00000073,
                    }
                    Memory32Fixed (ReadWrite,
                        0x05000000,         // Address Base
                        0x0003F010,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x05060000,         // Address Base
                        0x0003F000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x0000014C,
                    }
                    Memory32Fixed (ReadWrite,
                        0x0B290000,         // Address Base
                        0x0000FFFF,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0B490000,         // Address Base
                        0x00010000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x05090000,         // Address Base
                        0x00009000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0508D000,         // Address Base
                        0x00001C00,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0C200000,         // Address Base
                        0x0000FFFF,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0AA00000,         // Address Base
                        0x00200000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x000000CE,
                    }
                })
                Name (BBUF, ResourceTemplate ()
                {
                    Memory32Fixed (ReadWrite,
                        0x0AE00000,         // Address Base
                        0x00151000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x088E0000,         // Address Base
                        0x000F4000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x00000073,
                    }
                    Memory32Fixed (ReadWrite,
                        0x05000000,         // Address Base
                        0x0003F010,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x05060000,         // Address Base
                        0x0003F000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x0000014C,
                    }
                    Memory32Fixed (ReadWrite,
                        0x0B290000,         // Address Base
                        0x0000FFFF,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0B490000,         // Address Base
                        0x00010000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x05090000,         // Address Base
                        0x00009000,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0508D000,         // Address Base
                        0x00001C00,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0C200000,         // Address Base
                        0x0000FFFF,         // Address Length
                        )
                    Memory32Fixed (ReadWrite,
                        0x0AA00000,         // Address Base
                        0x00200000,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x000000CE,
                    }
                })
                If ((BSID () == Zero))
                {
                    Return (ABUF) /* \_SB_.GPU0._CRS.ABUF */
                }
                Else
                {
                    Return (BBUF) /* \_SB_.GPU0._CRS.BBUF */
                }
            }
```

## DSDT.dsl:43303

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If (((\_SB.PLST == One) && (BSID () == 0x03)))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0B)
                }
            }
```

## DSDT.dsl:43349

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:43390

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:44927

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If (((\_SB.PLST == One) && ((BSID () == One) || (BSID () == 0x03))))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:45120

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:45135

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:45150

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:45165

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:45180

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```

## DSDT.dsl:47569

```asl
Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((BREV () == One))
                {
                    Return (Zero)
                }
                Else
                {
                    Return (0x0F)
                }
            }
```
