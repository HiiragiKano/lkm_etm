#ifndef _FUNNEL_LANRAN_H
#define _FUNNEL_LANRAN_H

#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>

#define CORESIGHT_UNLOCK 0xc5acce55
#define CORESIGHT_LAR 0xfb0

#define FUNNEL_FUNCTL 0x000
#define FUNNEL_PRICTL 0x004

#define FUNNEL_HOLDTIME_MASK 0xf00
#define FUNNEL_HOLDTIME_SHFT 0x8
#define FUNNEL_HOLDTIME (0x7 << FUNNEL_HOLDTIME_SHFT)

struct funnel_drvdata
{
    void __iomem *base;
    struct device *dev;
    struct clk *atclk;
    struct coresight_device *csdev;
    unsigned long priority;
};

static inline void CS_UNLOCK(void __iomem *addr)
{
    do
    {
        writel_relaxed(CORESIGHT_UNLOCK, addr + CORESIGHT_LAR);
        /* Make sure everyone has seen this */
        mb();
    } while (0);
}

static inline void CS_LOCK(void __iomem *addr)
{
    do
    {
        /* Wait for things to settle */
        mb();
        writel_relaxed(0x0, addr + CORESIGHT_LAR);
    } while (0);
}

static void funnel_enable_hw(struct funnel_drvdata *drvdata, int port)
{
    u32 functl;

    CS_UNLOCK(drvdata->base);

    functl = readl_relaxed(drvdata->base + FUNNEL_FUNCTL);
    functl &= ~FUNNEL_HOLDTIME_MASK;
    functl |= FUNNEL_HOLDTIME;

    // port = 0
    functl |= (1 << port);
    writel_relaxed(functl, drvdata->base + FUNNEL_FUNCTL);
    // writel_relaxed(drvdata->priority, drvdata->base + FUNNEL_PRICTL);

    CS_LOCK(drvdata->base);
}

static void funnel_disable_hw(struct funnel_drvdata *drvdata, int inport)
{
    u32 functl;

    CS_UNLOCK(drvdata->base);

    functl = readl_relaxed(drvdata->base + FUNNEL_FUNCTL);
    functl &= ~(1 << inport);
    writel_relaxed(functl, drvdata->base + FUNNEL_FUNCTL);

    CS_LOCK(drvdata->base);
}
#endif