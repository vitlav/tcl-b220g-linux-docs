# Windows PEP / Linux SC7180 UFS register comparison

PE string-pointer records contain the listed MMIO constants. Structure fields and runtime usage are not fully decoded. Equality of constants does not prove identical sequencing. Linux GCC base: 0x100000.

|PEP name|Linux name|MMIO|Record VA|Match in first 8 qwords|
|---|---|---|---|---|
|gcc_aggre_ufs_phy_axi_clk|gcc_aggre_ufs_phy_axi_clk|0x182024|0x140178390|True|
|gcc_ufs_phy_ahb_clk|gcc_ufs_phy_ahb_clk|0x177014|0x140176e90|True|
|gcc_ufs_phy_axi_clk|gcc_ufs_phy_axi_clk|0x177038|0x1401783f0|True|
|gcc_ufs_phy_ice_core_clk|gcc_ufs_phy_ice_core_clk|0x177090|0x140178450|True|
|gcc_ufs_phy_phy_aux_clk|gcc_ufs_phy_phy_aux_clk|0x177094|0x1401784b0|True|
|gcc_ufs_phy_rx_symbol_0_clk|gcc_ufs_phy_rx_symbol_0_clk|0x17701c|0x140178510|True|
|gcc_ufs_phy_rx_symbol_0_clk|gcc_ufs_phy_rx_symbol_0_clk|0x17701c|0x14017fca8|False|
|gcc_ufs_phy_tx_symbol_0_clk|gcc_ufs_phy_tx_symbol_0_clk|0x177018|0x140178570|True|
|gcc_ufs_phy_tx_symbol_0_clk|gcc_ufs_phy_tx_symbol_0_clk|0x177018|0x14017fcb0|False|
|gcc_ufs_phy_unipro_core_clk|gcc_ufs_phy_unipro_core_clk|0x17708c|0x1401785d0|True|
|gcc_ufs_mem_clkref_en|gcc_ufs_mem_clkref_clk|0x18c000|0x140175690|True|
|gcc_ufs_phy_gdsc|ufs_phy_gdsc|0x177004|0x14017eae0|True|
