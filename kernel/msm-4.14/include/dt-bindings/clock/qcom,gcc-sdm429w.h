/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#ifndef _DT_BINDINGS_CLK_QCOM_GCC_SDM429W_H
#define _DT_BINDINGS_CLK_QCOM_GCC_SDM429W_H

#define GPLL0_OUT_MAIN			0
#define GPLL0_AO_CLK_SRC			1
#define GPLL1_OUT_MAIN			2
#define GPLL3_OUT_MAIN			3
#define GPLL4_OUT_MAIN			4
#define GPLL0_AO_OUT_MAIN			5
#define GPLL0_SLEEP_CLK_SRC			6
#define GPLL6			7
#define GPLL6_OUT_AUX			8
#define APSS_AHB_CLK_SRC			9
#define BLSP1_QUP1_I2C_APPS_CLK_SRC			10
#define BLSP1_QUP1_SPI_APPS_CLK_SRC			11
#define BLSP1_QUP2_I2C_APPS_CLK_SRC			12
#define BLSP1_QUP2_SPI_APPS_CLK_SRC			13
#define BLSP1_QUP3_I2C_APPS_CLK_SRC			14
#define BLSP1_QUP3_SPI_APPS_CLK_SRC			15
#define BLSP1_QUP4_I2C_APPS_CLK_SRC			16
#define BLSP1_QUP4_SPI_APPS_CLK_SRC			17
#define BLSP1_UART1_APPS_CLK_SRC			18
#define BLSP1_UART2_APPS_CLK_SRC			19
#define BLSP2_QUP1_I2C_APPS_CLK_SRC			20
#define BLSP2_QUP1_SPI_APPS_CLK_SRC			21
#define BLSP2_QUP2_I2C_APPS_CLK_SRC			22
#define BLSP2_QUP2_SPI_APPS_CLK_SRC			23
#define BLSP2_QUP3_I2C_APPS_CLK_SRC			24
#define BLSP2_QUP3_SPI_APPS_CLK_SRC			25
#define BLSP2_QUP4_I2C_APPS_CLK_SRC			26
#define BLSP2_QUP4_SPI_APPS_CLK_SRC			27
#define BLSP2_UART1_APPS_CLK_SRC			28
#define BLSP2_UART2_APPS_CLK_SRC			29
#define BYTE0_CLK_SRC			30
#define BYTE1_CLK_SRC			31
#define CAMSS_TOP_AHB_CLK_SRC			32
#define CCI_CLK_SRC			33
#define CPP_CLK_SRC			34
#define CRYPTO_CLK_SRC			35
#define CSI0_CLK_SRC			36
#define CSI0PHYTIMER_CLK_SRC			37
#define CSI1_CLK_SRC			38
#define CSI1PHYTIMER_CLK_SRC			39
#define CSI2_CLK_SRC			40
#define ESC0_CLK_SRC			41
#define ESC1_CLK_SRC			42
#define GCC_BIMC_GFX_CLK			43
#define GCC_BIMC_GPU_CLK			44
#define GCC_BLSP1_AHB_CLK			45
#define GCC_BLSP1_QUP1_I2C_APPS_CLK			46
#define GCC_BLSP1_QUP1_SPI_APPS_CLK			47
#define GCC_BLSP1_QUP2_I2C_APPS_CLK			48
#define GCC_BLSP1_QUP2_SPI_APPS_CLK			49
#define GCC_BLSP1_QUP3_I2C_APPS_CLK			50
#define GCC_BLSP1_QUP3_SPI_APPS_CLK			51
#define GCC_BLSP1_QUP4_I2C_APPS_CLK			52
#define GCC_BLSP1_QUP4_SPI_APPS_CLK			53
#define GCC_BLSP1_UART1_APPS_CLK			54
#define GCC_BLSP1_UART2_APPS_CLK			55
#define GCC_BLSP2_QUP1_I2C_APPS_CLK			56
#define GCC_BLSP2_QUP1_SPI_APPS_CLK			57
#define GCC_BLSP2_QUP2_I2C_APPS_CLK			58
#define GCC_BLSP2_QUP2_SPI_APPS_CLK			59
#define GCC_BLSP2_QUP3_I2C_APPS_CLK			60
#define GCC_BLSP2_QUP3_SPI_APPS_CLK			61
#define GCC_BLSP2_QUP4_I2C_APPS_CLK			62
#define GCC_BLSP2_QUP4_SPI_APPS_CLK			63
#define GCC_BLSP2_UART1_APPS_CLK			64
#define GCC_BLSP2_UART2_APPS_CLK			65
#define GCC_BLSP2_AHB_CLK			66
#define GCC_BOOT_ROM_AHB_CLK			67
#define GCC_CAMSS_AHB_CLK			68
#define GCC_CAMSS_CCI_AHB_CLK			69
#define GCC_CAMSS_CCI_CLK			70
#define GCC_CAMSS_CPP_AHB_CLK			71
#define GCC_CAMSS_CPP_AXI_CLK			72
#define GCC_CAMSS_CPP_CLK			73
#define GCC_CAMSS_CSI0_AHB_CLK			74
#define GCC_CAMSS_CSI0_CLK			75
#define GCC_CAMSS_CSI0PHY_CLK			76
#define GCC_CAMSS_CSI0PIX_CLK			77
#define GCC_CAMSS_CSI0RDI_CLK			78
#define GCC_CAMSS_CSI1_AHB_CLK			79
#define GCC_CAMSS_CSI1_CLK			80
#define GCC_CAMSS_CSI1PHY_CLK			81
#define GCC_CAMSS_CSI1PIX_CLK			82
#define GCC_CAMSS_CSI1RDI_CLK			83
#define GCC_CAMSS_CSI2_AHB_CLK			84
#define GCC_CAMSS_CSI2_CLK			85
#define GCC_CAMSS_CSI2PHY_CLK			86
#define GCC_CAMSS_CSI2PIX_CLK			87
#define GCC_CAMSS_CSI2RDI_CLK			88
#define GCC_CAMSS_CSI_VFE0_CLK			89
#define GCC_CAMSS_GP0_CLK_SRC			90
#define GCC_CAMSS_GP1_CLK_SRC			91
#define GCC_CAMSS_CSI_VFE1_CLK			92
#define GCC_CAMSS_CSI0PHYTIMER_CLK			93
#define GCC_CAMSS_CSI1PHYTIMER_CLK			94
#define GCC_CAMSS_GP0_CLK			95
#define GCC_CAMSS_GP1_CLK			96
#define GCC_CAMSS_ISPIF_AHB_CLK			97
#define GCC_CAMSS_JPEG0_CLK			98
#define GCC_CAMSS_JPEG_AHB_CLK			99
#define GCC_CAMSS_JPEG_AXI_CLK			100
#define GCC_CAMSS_MCLK0_CLK			101
#define GCC_CAMSS_MCLK1_CLK			102
#define GCC_CAMSS_MCLK2_CLK			103
#define GCC_CAMSS_MICRO_AHB_CLK			104
#define GCC_CAMSS_TOP_AHB_CLK			105
#define GCC_CAMSS_VFE0_CLK			106
#define GCC_CAMSS_VFE1_AHB_CLK			107
#define GCC_CAMSS_VFE1_AXI_CLK			108
#define GCC_CAMSS_VFE1_CLK			109
#define GCC_CAMSS_VFE_AHB_CLK			110
#define GCC_CAMSS_VFE_AXI_CLK			111
#define GCC_CRYPTO_AHB_CLK			112
#define GCC_CRYPTO_AXI_CLK			113
#define GCC_CRYPTO_CLK			114
#define GCC_DCC_CLK			115
#define GCC_GP1_CLK			116
#define GCC_GP2_CLK			117
#define GCC_GP3_CLK			118
#define GCC_MDSS_AHB_CLK			119
#define GCC_MDSS_AXI_CLK			120
#define GCC_MDSS_BYTE0_CLK			121
#define GCC_MDSS_BYTE1_CLK			122
#define GCC_MDSS_ESC0_CLK			123
#define GCC_MDSS_ESC1_CLK			124
#define GCC_MDSS_MDP_CLK			125
#define GCC_MDSS_PCLK0_CLK			126
#define GCC_MDSS_PCLK1_CLK			127
#define GCC_MDSS_VSYNC_CLK			128
#define GCC_MSS_CFG_AHB_CLK			129
#define GCC_MSS_Q6_BIMC_AXI_CLK			130
#define GCC_OXILI_AHB_CLK			131
#define GCC_OXILI_AON_CLK			132
#define GCC_OXILI_GFX3D_CLK			133
#define GCC_OXILI_TIMER_CLK			134
#define GCC_PDM2_CLK				135
#define GCC_PDM_AHB_CLK			136
#define GCC_PRNG_AHB_CLK			137
#define GCC_SDCC1_AHB_CLK			138
#define GCC_SDCC1_APPS_CLK			139
#define GCC_SDCC1_ICE_CORE_CLK			140
#define GCC_SDCC2_AHB_CLK			141
#define GCC_SDCC2_APPS_CLK			142
#define GCC_USB2A_PHY_SLEEP_CLK				143
#define GCC_USB_HS_AHB_CLK			144
#define GCC_USB_HS_PHY_CFG_AHB_CLK			145
#define GCC_USB_HS_SYSTEM_CLK			146
#define GCC_VENUS0_AHB_CLK			147
#define GCC_VENUS0_AXI_CLK			148
#define GCC_VENUS0_CORE0_VCODEC0_CLK			149
#define GCC_VENUS0_VCODEC0_CLK			150
#define GCC_XO_CLK_SRC			151
#define GFX3D_CLK_SRC			152
#define GP1_CLK_SRC			153
#define GP2_CLK_SRC			154
#define GP3_CLK_SRC			155
#define JPEG0_CLK_SRC			156
#define MCLK0_CLK_SRC			157
#define MCLK1_CLK_SRC			158
#define MCLK2_CLK_SRC			159
#define MDP_CLK_SRC			160
#define MDSS_MDP_VOTE_CLK			161
#define MDSS_ROTATOR_VOTE_CLK			162
#define PCLK0_CLK_SRC			163
#define PCLK1_CLK_SRC			164
#define PDM2_CLK_SRC			165
#define SDCC1_APPS_CLK_SRC			166
#define SDCC1_ICE_CORE_CLK_SRC			167
#define SDCC2_APPS_CLK_SRC			168
#define USB_HS_SYSTEM_CLK_SRC			169
#define VCODEC0_CLK_SRC			170
#define VFE0_CLK_SRC			171
#define VFE1_CLK_SRC			172
#define VSYNC_CLK_SRC			173
#define GCC_APSS_TCU_CLK		174
#define GCC_CPP_TBU_CLK			175
#define GCC_JPEG_TBU_CLK		176
#define GCC_MDP_TBU_CLK			177
#define GCC_SMMU_CFG_CLK		178
#define GCC_VENUS_TBU_CLK		179
#define GCC_VFE_TBU_CLK			180
#define GCC_VFE1_TBU_CLK		181
#define GCC_QDSS_DAP_CLK		182

/* GCC resets */
#define GCC_CAMSS_MICRO_BCR			0
#define GCC_USB_FS_BCR			1
#define GCC_USB_HS_BCR			2
#define GCC_USB2_HS_PHY_ONLY_BCR			3
#define GCC_QUSB2_PHY_BCR			4

#endif
