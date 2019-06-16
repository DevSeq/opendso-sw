#include <stdio.h>
#include "ual.h"
#include "dsi_core.h"

#define BASE_DSI 0x60000000

static struct ual_bar_tkn *ubar;

void dsi_write(uint32_t reg, uint32_t val)
{
    //printf("DSI write %x %x\n", reg, val);
    ual_writel(ubar, reg , val);
}

uint32_t dsi_read(uint32_t reg)
{
    return ual_readl(ubar, reg );
}

/*1              -> l2
clk            -> clk
2              -> l1
3              -> l0*/


struct dsi_panel_config panel_iphone4 = {
  "LH350WS02 (Iphone4/4S)",
  3,    /* num_lanes */

    DSI_LANE(0, 0, 0) |
    DSI_LANE(1, 2, 0) |
    DSI_LANE(2, 1, 0) |
    DSI_LANE(3, 0, 0) | DSI_LANE_CLOCK_POLARITY, /* Lane configuration */

  2,    /* lp_divider */
  640,  /* width */
  960, /* height */

  192,   /* h_front_porch */
  192,   /* h_back_porch */
  10,   /* v_front_porch */
  28,    /* v_back_porch */
  10000,  /* frame_gap */
  100000,
  NULL,  /* EDID */
  NULL,
  NULL,		/* user init */
};

main()
{
	struct ual_desc_rawmem rawmem;

    rawmem.offset = BASE_DSI;
    rawmem.size = 0x10000;

    ubar = ual_open(UAL_BUS_RAWMEM, &rawmem);

//    printf("ubar %p\n", ubar);
    //ual_writel( ubar, REG_H_FRONT_PORCH, 123 );
    //printf("Rdbk %x\n", ual_readl(ubar, REG_DSI_GPIO) );

    dsi_init(&panel_iphone4);
 
    //return 0;
    usleep(1);
   // dsi_force_lp(1);
    dsi_write( REG_TEST_CTL, 0);
    dsi_write( REG_TEST_XSIZE, 640-1);
    dsi_write( REG_TEST_YSIZE, 960-1);
    //getchar();
    dsi_write( REG_TEST_CTL, 1);
    //dsi_force_lp(0);
    
    return 0;
}