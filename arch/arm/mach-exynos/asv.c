/* linux/arch/arm/mach-exynos4/asv.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS4 - ASV(Adaptive Supply Voltage) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/mach-types.h>

#include <mach/map.h>
#include <mach/regs-iem.h>
#include <mach/regs-pmu.h>
#include <mach/regs-clock.h>
#include <mach/cpufreq.h>

#define LOOP_CNT	10

static int __init iem_clock_init(void)
{
	struct clk *clk_hpm;
	struct clk *clk_copy;
	struct clk *clk_parent;

	/* PWI clock setting */
	clk_copy = clk_get(NULL, "sclk_pwi");
	if (IS_ERR(clk_copy)) {
		printk(KERN_ERR"ASV : SCLK_PWI clock get error\n");
		return -EINVAL;
	} else {
		clk_parent = clk_get(NULL, "xusbxti");

		if (IS_ERR(clk_parent)) {
			printk(KERN_ERR"ASV : MOUT_APLL clock get error\n");
			clk_put(clk_copy);
			return -EINVAL;
		}
		clk_set_parent(clk_copy, clk_parent);

		clk_put(clk_parent);
	}
	clk_set_rate(clk_copy, 4800000);

	clk_put(clk_copy);

	/* HPM clock setting */
	clk_copy = clk_get(NULL, "dout_copy");
	if (IS_ERR(clk_copy)) {
		printk(KERN_ERR"ASV : DOUT_COPY clock get error\n");
		return -EINVAL;
	} else {
		clk_parent = clk_get(NULL, "mout_mpll");
		if (IS_ERR(clk_parent)) {
			printk(KERN_ERR"ASV : MOUT_APLL clock get error\n");
			clk_put(clk_copy);
			return -EINVAL;
		}
		clk_set_parent(clk_copy, clk_parent);

		clk_put(clk_parent);
	}

	clk_set_rate(clk_copy, (400 * 1000 * 1000));

	clk_put(clk_copy);

	clk_hpm = clk_get(NULL, "sclk_hpm");
	if (IS_ERR(clk_hpm))
		return -EINVAL;

	clk_set_rate(clk_hpm, (200 * 1000 * 1000));

	clk_put(clk_hpm);

	return 0;
}

void __init iem_clock_set(void)
{
	/* APLL_CON0 level register */
	__raw_writel(0x80FA0601, S5P_APLL_CON0L8);
	__raw_writel(0x80C80601, S5P_APLL_CON0L7);
	__raw_writel(0x80C80602, S5P_APLL_CON0L6);
	__raw_writel(0x80C80604, S5P_APLL_CON0L5);
	__raw_writel(0x80C80601, S5P_APLL_CON0L4);
	__raw_writel(0x80C80601, S5P_APLL_CON0L3);
	__raw_writel(0x80C80601, S5P_APLL_CON0L2);
	__raw_writel(0x80C80601, S5P_APLL_CON0L1);

	/* IEM Divider register */
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L8);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L7);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L6);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L5);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L4);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L3);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L2);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L1);
}

#define IDS_OFFSET			24
#define IDS_MASK			0xFF

#define IDS_LEVEL_MAX_1400_0		8
#define IDS_LEVEL_MAX_1400_1		15
#define IDS_LEVEL_MAX_1400_2		37
#define IDS_LEVEL_MAX_1400_3		52

#define HPM_LEVEL_MAX_1400_0		13
#define HPM_LEVEL_MAX_1400_1		17
#define HPM_LEVEL_MAX_1400_2		22
#define HPM_LEVEL_MAX_1400_3		26

#define IDS_LEVEL_MAX_1200_0		4
#define IDS_LEVEL_MAX_1200_1		8
#define IDS_LEVEL_MAX_1200_2		12
#define IDS_LEVEL_MAX_1200_3		17
#define IDS_LEVEL_MAX_1200_4		27
#define IDS_LEVEL_MAX_1200_5		45
#define IDS_LEVEL_MAX_1200_6		55

#define HPM_LEVEL_MAX_1200_0		8
#define HPM_LEVEL_MAX_1200_1		11
#define HPM_LEVEL_MAX_1200_2		14
#define HPM_LEVEL_MAX_1200_3		18
#define HPM_LEVEL_MAX_1200_4		21
#define HPM_LEVEL_MAX_1200_5		23
#define HPM_LEVEL_MAX_1200_6		25

enum find_asv {
	HPM_GROUP,
	IDS_GROUP,
};

static int __init exynos4_find_group_1200(unsigned int value, enum find_asv grp)
{
	unsigned int ret;

	if (grp == HPM_GROUP) {
		if (value <= HPM_LEVEL_MAX_1200_0)
			ret = 0;
		else if (value <= HPM_LEVEL_MAX_1200_1)
			ret = 1;
		else if (value <= HPM_LEVEL_MAX_1200_2)
			ret = 2;
		else if (value <= HPM_LEVEL_MAX_1200_3)
			ret = 3;
		else if (value <= HPM_LEVEL_MAX_1200_4)
			ret = 4;
		else if (value <= HPM_LEVEL_MAX_1200_5)
			ret = 5;
		else
			ret = 6;
	} else {
		if (value <= IDS_LEVEL_MAX_1200_0)
			ret = 0;
		else if (value <= IDS_LEVEL_MAX_1200_1)
			ret = 1;
		else if (value <= IDS_LEVEL_MAX_1200_2)
			ret = 2;
		else if (value <= IDS_LEVEL_MAX_1200_3)
			ret = 3;
		else if (value <= IDS_LEVEL_MAX_1200_4)
			ret = 4;
		else if (value <= IDS_LEVEL_MAX_1200_5)
			ret = 5;
		else
			ret = 6;
	}

	return ret;
}

static int __init exynos4_find_group_1400(unsigned int value, enum find_asv grp)
{
	unsigned int ret;

	if (grp == HPM_GROUP) {
		if (value <= HPM_LEVEL_MAX_1400_0)
			ret = 0;
		else if (value <= HPM_LEVEL_MAX_1400_1)
			ret = 1;
		else if (value <= HPM_LEVEL_MAX_1400_2)
			ret = 2;
		else if (value <= HPM_LEVEL_MAX_1400_3)
			ret = 3;
		else
			ret = 4;
	} else {
		if (value <= IDS_LEVEL_MAX_1400_0)
			ret = 0;
		else if (value <= IDS_LEVEL_MAX_1400_1)
			ret = 1;
		else if (value <= IDS_LEVEL_MAX_1400_2)
			ret = 2;
		else if (value <= IDS_LEVEL_MAX_1400_3)
			ret = 3;
		else
			ret = 4;
	}

	return ret;
}

static int __init exynos4_asv_init(void)
{
	unsigned int i;
	unsigned long hpm_delay = 0;
	unsigned int tmp;
	unsigned int ids_arm;
	unsigned int result_group = 0;
	bool for_1400 = false, for_1200 = false, for_1000 = false;
	void __iomem *iem_base;
	char *freq;

	if (machine_is_smdkv310())
		goto out;

	tmp = __raw_readl(S5P_VA_CHIPID + 0x4);

	/* Check CHIPID[2:0] bit field */
	switch (tmp & 0x7) {
	case 0:
	case 3:
		for_1000 = true;
		freq = "1GHz";
		break;
	case 1:
	case 7:
		for_1200 = true;
		freq = "1.2GHz";
		break;
	case 5:
		for_1400 = true;
		freq = "1.4GHz";
		break;
	default:
		printk(KERN_ERR "can't find Chip type[0x%x]\n", tmp);
		for_1000 = true;
		freq = "1GHz";
		break;
	}
	printk(KERN_INFO "Support %s\n", freq);

	ids_arm = ((tmp >> IDS_OFFSET) & IDS_MASK);

	iem_base = ioremap(EXYNOS4_PA_IEM, (128 * 1024));

	if (iem_base == NULL) {
		printk(KERN_ERR "faile to ioremap\n");
		goto out;
	}

	if (iem_clock_init()) {
		printk(KERN_ERR "ASV driver clock_init fail\n");
		goto out;
	} else {
		/* HPM enable  */
		tmp = __raw_readl(iem_base + EXYNOS4_APC_CONTROL);
		tmp |= APC_HPM_EN;
		__raw_writel(tmp, (iem_base + EXYNOS4_APC_CONTROL));

		iem_clock_set();

		/* IEM enable */
		tmp = __raw_readl(iem_base + EXYNOS4_IECDPCCR);
		tmp |= IEC_EN;
		__raw_writel(tmp, (iem_base + EXYNOS4_IECDPCCR));
	}

	for (i = 0 ; i < LOOP_CNT ; i++) {
		tmp = __raw_readb(iem_base + EXYNOS4_APC_DBG_DLYCODE);
		hpm_delay += tmp;
	}

	hpm_delay /= LOOP_CNT;

	if (for_1400) {
		result_group = exynos4_find_group_1400(hpm_delay, HPM_GROUP);

		if (result_group > exynos4_find_group_1400(ids_arm, IDS_GROUP))
			result_group = exynos4_find_group_1400(ids_arm, IDS_GROUP);

		result_group |= SUPPORT_1400MHZ;
	} else {
		result_group = exynos4_find_group_1200(hpm_delay, HPM_GROUP);

		if (result_group > exynos4_find_group_1200(ids_arm, IDS_GROUP))
			result_group = exynos4_find_group_1200(ids_arm, IDS_GROUP);

		if (for_1200)
			result_group |= SUPPORT_1200MHZ;
		else
			result_group |= SUPPORT_1000MHZ;
	}

	__raw_writel(result_group, S5P_INFORM2);

	printk(KERN_INFO "ASV Group for This Exynos4210 is 0x%x\n", result_group);

	return 0;

out:
	__raw_writel(0, S5P_INFORM2);
	printk(KERN_INFO "ASV Group for This Exynos4210 is %d\n", result_group);

	return -EINVAL;

}
core_initcall(exynos4_asv_init);
