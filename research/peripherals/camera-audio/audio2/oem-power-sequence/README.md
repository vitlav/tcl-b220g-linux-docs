# TCL OEM audio power prerequisites — 2026-09-09

Confirmed by preserved board ACPI and static OEM driver analysis, then read-only CMD DB probe on live Linux. No power votes or GPIO writes performed.

## OEM board tables

DSDT PEP0.APMD returns APCC in BOTH BSID branches; APCD is not selected by that method. APCC AUDD contains24 components. Do not use the pin-empty APCD component5/6 definitions as the active board configuration.

| AUDD component | APCC active PSTATE0 resource | Linux correspondence |
|---|---|---|
| 1 | PPP_RESOURCE_ID_BUCK_BOOST1_C, raw voltage0x324b00=3300000 | PM6150L id c, resource bobc1, node bob |
| 2 | PPP_RESOURCE_ID_LDO15_A, raw voltage0x1b7740=1800000 | PM6150 id a, resource ldoa15, node ldo15 |
| 5 | TLMMGPIO entries0,1,2,14 | TX LPI pin group; raw fields retained in APCC.dsl.txt |
| 6 | TLMMGPIO entries3,4,5 | RX LPI pin group, matches current Linux mux ownership |
| 3,4 | no explicit resource package | Empty table does not prove these components have no firmware-side effects |

APCC PSTATE1 clears enable in regulator tuples. Do not assume raw PMIC mode numbers map to Linux enum values. No audio LDO10_A vote appears in this APCC component list; do not copy Atoll LDO10 supply based on name alone.

## Driver path

Prerequisites140081b48 called with indices0and1 before reset construct separate bus contexts, synchronization objects and callbacks;14007ead0 then ORs masks0x3f and0x3f00 into register6295a000. It issues outer callback7 with payload(enable1,selector1) for index0, and(enable1,selector0) for index1. Callback14003d468 maps these to AUDD component5 and6. Helper14005f410 goes via140054d88 to IAT140018218=PoFxActivateComponent, then140018250=PoFxIssueComponentPerfStateChange. Import identities independently checked against saved PE IAT order. These are pin-group power-state requests, not external reset GPIO writes.

Immediately preceding the reset pulse,14006ac40 sends outer callback1 through14006a0c8. Handler14003d2e0 maps four change fields to components4,3,2,1 (offsets12,0,4,8), using values1=activate,-1=deactivate,0=no change. Thus the callback path reaches the board's LDO15/BOB components; exact which change fields are nonzero depends on prior runtime state. Do not claim every reset call necessarily enables both rails anew.

## Live comparison and CMD DB probe

Current DT regulator banks are pm6150 id a and pm6150l id c. Neither ldo15 nor bob registered in regulator_summary. This means Linux has no consumer votes for those providers in this configuration; it does NOT prove that firmware has electrically disabled the rails.

Read-only module compiled against6.18.34-tcl-audio2, loaded and normally unloaded on boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Resources ldoa15=0x42200,bobc1=0x40400 are present, type4(VRM), auxiliary data4bytes. ldoa10=0x41d00 and ldoc10=0x41900 confirm A/C are different resources. No RPMh request issued. Taint remains4096. Code and exact output saved here.

Current mux ownership confirms RX gpio3/4/5 assigned to rx-pin-test. Pinconf debugfs displays empty strings, so it cannot independently verify every electrical setting. SoundWire still only master; no sound confirmed.

## Voltage constraint before hardware test

Kernel qcom-rpmh-regulator.c pmic5_pldo_lv table:1504000+8000*n;1800000 is representable (n37). pmic5_bob:3000000+32000*n;3300000 is NOT representable. Neighbours3288000/3320000. Exact min=max3300000 would reject voltage selection. Do not silently round this to3320000 or expand the allowed voltage range without establishing codec tolerance or OEM PMIC rounding behavior. CMD DB presence alone does not establish such tolerance.

Next: identify OEM BOB voltage rounding/range and remaining codec supply path; prepare regulator providers with confirmed constraints. Resolve GPIO58 ownership/tile independently before reset experiment. Reserved58..62 stay excluded. No reboot, bootfile edits, or Bugzilla publication.
