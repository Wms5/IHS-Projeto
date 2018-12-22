
module pcihellocore (
	button0_external_connection_export,
	cool_external_connection_export,
	hexport_external_connection_export,
	inport_external_connection_export,
	ledextra_external_connection_export,
	ledverde_external_connection_export,
	ledvermelho_external_connection_export,
	pcie_hard_ip_0_pcie_rstn_export,
	pcie_hard_ip_0_powerdown_pll_powerdown,
	pcie_hard_ip_0_powerdown_gxb_powerdown,
	pcie_hard_ip_0_refclk_export,
	pcie_hard_ip_0_rx_in_rx_datain_0,
	pcie_hard_ip_0_tx_out_tx_dataout_0);	

	input	[15:0]	button0_external_connection_export;
	output	[31:0]	cool_external_connection_export;
	output	[31:0]	hexport_external_connection_export;
	input	[15:0]	inport_external_connection_export;
	output	[31:0]	ledextra_external_connection_export;
	output	[31:0]	ledverde_external_connection_export;
	output	[31:0]	ledvermelho_external_connection_export;
	input		pcie_hard_ip_0_pcie_rstn_export;
	input		pcie_hard_ip_0_powerdown_pll_powerdown;
	input		pcie_hard_ip_0_powerdown_gxb_powerdown;
	input		pcie_hard_ip_0_refclk_export;
	input		pcie_hard_ip_0_rx_in_rx_datain_0;
	output		pcie_hard_ip_0_tx_out_tx_dataout_0;
endmodule
