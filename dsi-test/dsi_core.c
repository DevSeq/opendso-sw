/*
 * DSI Core
 * Copyright (C) 2013-2015 twl <twlostow@printf.cc>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

/* dsi_core.c - main DSI core driver */

#include <stdio.h>
#include <stdint.h>

#include "dsi_core.h"

#include <stdio.h>
#include <stdint.h>


/* Calculates a parity bit for value d (1 = odd, 0 = even) */
static inline uint8_t parity(uint32_t d)
{
    int i, p = 0;

    for (i = 0; i < 32; i++)
        p ^= d & (1 << i) ? 1 : 0;
    return p;
}

void delay(int tics)
{
	while(tics--) asm volatile("nop");
}

static uint8_t reverse_bits(uint8_t x)
{
    uint8_t r = 0;
    int     i;

    for (i = 0; i < 8; i++)
        if (x & (1 << i)) r |= (1 << (7 - i));
    return r;
}

/* calculates DSI packet header ECC checksum */
static uint8_t dsi_ecc(uint32_t data)
{
    uint8_t ecc = 0;
    int     i;
    static const uint32_t masks[] =
    { 0xf12cb7, 0xf2555b, 0x749a6d, 0xb8e38e, 0xdf03f0, 0xeffc00 };

    for (i = 0; i < 6; i++)
        if (parity(data & masks[i]))
            ecc |= (1 << i);

    return ecc;
}

int dsi_ctl = 0;

/* Sends a single byte to the display in low power mode */
void dsi_lp_write_byte(uint32_t value)
{
    int rv = 0;

    dsi_write(REG_DSI_CTL, dsi_ctl | 2);

    while (!(dsi_read(REG_DSI_CTL) & 2)) ;
    dsi_write(REG_LP_TX, value | 0x100);

    while (!(dsi_read(REG_DSI_CTL) & 2)) ;
}

/* Composes a short packet and sends it in low power mode to the display */
void dsi_send_lp_short(uint8_t ptype, uint8_t w0, uint8_t w1)
{
    uint8_t  pdata[4];
    uint32_t d;

    dsi_lp_write_byte(0xe1);
    dsi_lp_write_byte(reverse_bits(ptype));
    dsi_lp_write_byte(reverse_bits(w0));
    dsi_lp_write_byte(reverse_bits(w1));
    dsi_lp_write_byte(reverse_bits(dsi_ecc(ptype |
                                           (((uint32_t)w0) <<
    8) | (((uint32_t)w1) << 16))));
    dsi_write(REG_DSI_CTL, dsi_ctl);
}

uint16_t dsi_crc(const uint8_t *d, int n)
{
    uint16_t poly = 0x8408;

    int byte_counter;
    int bit_counter;
    uint8_t  current_data;
    uint16_t result = 0xffff;

    for (byte_counter = 0; byte_counter < n; byte_counter++) {
        current_data = d[byte_counter];

        for (bit_counter = 0; bit_counter < 8; bit_counter++)
        {
            if (((result & 0x0001) ^ ((current_data) & 0x0001)))
                result = ((result >> 1) & 0x7fff) ^ poly;
            else
                result = ((result >> 1) & 0x7fff);
            current_data = (current_data >> 1); // & 0x7F;
        }
    }
    return result;
}

void dsi_long_write(int is_dcs, const unsigned char *data, int length)
{
    uint8_t w1 = 0;
    uint8_t w0 = length;

    uint8_t ptype = is_dcs ? 0x39 : 0x29;
    //printf("pp_long write: %d bytes ptype %x\n", length, ptype);

    dsi_lp_write_byte(0xe1);
    dsi_lp_write_byte(reverse_bits(ptype));
    dsi_lp_write_byte(reverse_bits(w0));
    dsi_lp_write_byte(reverse_bits(w1));
    dsi_lp_write_byte(reverse_bits(dsi_ecc(ptype |
                                           (((uint32_t)w0) <<
    8) | (((uint32_t)w1) << 16))));

    int i;

    for (i = 0; i < length; i++)
        dsi_lp_write_byte(reverse_bits(data[i]));

    uint16_t crc = dsi_crc(data, length);

    crc = 0x0000;

    dsi_lp_write_byte(reverse_bits(crc & 0xff));
    dsi_lp_write_byte(reverse_bits(crc >> 8));
    dsi_write(REG_DSI_CTL, dsi_ctl);
}

void dsi_delay()
{
    usleep(100000);
}

void SSD_Single(uint8_t r, uint8_t data )
{
    uint8_t ddd[2] = {r,data};
    
    printf("ssd_single write: %x %x\n", r, data);
    dsi_send_lp_short( 0x13, r ,data );
    //dsi_long_write(1, ddd, 2);//int is_dcs, const unsigned char *data, int length)
    return;
    uint8_t w1 = data;
    uint8_t w0 = r;

    uint8_t ptype = 0x15; // dcs short write
    
    dsi_lp_write_byte(0xe1);
    dsi_lp_write_byte(reverse_bits(ptype));
    dsi_lp_write_byte(reverse_bits(w0));
    dsi_lp_write_byte(reverse_bits(w1));
    dsi_lp_write_byte(reverse_bits(dsi_ecc(ptype |
                                           (((uint32_t)w0) <<
    8) | (((uint32_t)w1) << 16))));
    uint16_t crc = 0x0000;

    dsi_lp_write_byte(reverse_bits(crc & 0xff));
    dsi_lp_write_byte(reverse_bits(crc >> 8));
    dsi_write(REG_DSI_CTL, dsi_ctl);
}

void dsi_init(struct dsi_panel_config *panel)
{
    int i;

    printf("LansCfg 0x%04x\n", panel->lane_config);

    dsi_write(REG_TIMING_CTL, 0); // disable core, force LP mode
    dsi_write(REG_DSI_LANE_CTL, panel->lane_config);
    dsi_write(REG_DSI_CTL, 0); // disable core
    dsi_write(REG_DSI_TICKDIV, panel->lp_divider);

    dsi_ctl = (panel->num_lanes << 8);
    dsi_write(REG_DSI_CTL, dsi_ctl); /* disable DSI clock, set lane count */

    printf("dsi_ctl %x\n", dsi_ctl );


    dsi_write(REG_DSI_GPIO, 0x2);    /* Avdd on */
    dsi_delay();
//    for (i = 0; i < 10; i++) dsi_delay();

    dsi_write(REG_DSI_GPIO, 0x1); /* reset the display */
    dsi_delay();

//    for (i = 0; i < 10; i++) dsi_delay();

    dsi_write(REG_DSI_GPIO, 0x3); /* un-reset */
    dsi_delay();

//    for (i = 0; i < 10; i++) dsi_delay();

  //  dsi_delay();

    dsi_ctl |= 1;
    usleep(100000);

    dsi_write(REG_DSI_CTL, dsi_ctl); /* enable DSI clock */
    dsi_delay();

    usleep(100000);

    printf("nop\n");
    dsi_send_lp_short(0x05, 0x00, 0x00); /* send DCS NOP */

    usleep(100000);
    

#if 0
SSD_Single(0xE0,0x00);

//--- PASSWORD  ----//
SSD_Single(0xE1,0x93);
SSD_Single(0xE2,0x65);
SSD_Single(0xE3,0xF8);


//Page0
SSD_Single(0xE0,0x00);
//--- Sequence Ctrl  ----//
SSD_Single(0x70,0x10);	//DC0,DC1
SSD_Single(0x71,0x13);	//DC2,DC3
SSD_Single(0x72,0x06);	//DC7
SSD_Single(0x80,0x03);	//0x03:4-Lane£»0x02:3-Lane
//--- Page4  ----//
SSD_Single(0xE0,0x04);
SSD_Single(0x2D,0x03);
//--- Page1  ----//
SSD_Single(0xE0,0x01);

//Set VCOM
SSD_Single(0x00,0x00);
SSD_Single(0x01,0xA0);
//Set VCOM_Reverse
SSD_Single(0x03,0x00);
SSD_Single(0x04,0xA0);

//Set Gamma Power, VGMP,VGMN,VGSP,VGSN
SSD_Single(0x17,0x00);
SSD_Single(0x18,0xB1);
SSD_Single(0x19,0x01);
SSD_Single(0x1A,0x00);
SSD_Single(0x1B,0xB1);  //VGMN=0
SSD_Single(0x1C,0x01);
                
//Set Gate Power
SSD_Single(0x1F,0x3E);     //VGH_R  = 15V                       
SSD_Single(0x20,0x2D);     //VGL_R  = -12V                      
SSD_Single(0x21,0x2D);     //VGL_R2 = -12V                      
SSD_Single(0x22,0x0E);     //PA[6]=0, PA[5]=0, PA[4]=0, PA[0]=0 

//SETPANEL
SSD_Single(0x37,0x19);	//SS=1,BGR=1

//SET RGBCYC
SSD_Single(0x38,0x05);	//JDT=101 zigzag inversion
SSD_Single(0x39,0x08);	//RGB_N_EQ1, modify 20140806
SSD_Single(0x3A,0x12);	//RGB_N_EQ2, modify 20140806
SSD_Single(0x3C,0x78);	//SET EQ3 for TE_H
SSD_Single(0x3E,0x80);	//SET CHGEN_OFF, modify 20140806 
SSD_Single(0x3F,0x80);	//SET CHGEN_OFF2, modify 20140806


//Set TCON
SSD_Single(0x40,0x06);	//RSO=800 RGB
SSD_Single(0x41,0xA0);	//LN=640->1280 line

//--- power voltage  ----//
SSD_Single(0x55,0x01);	//DCDCM=0001, JD PWR_IC
SSD_Single(0x56,0x01);
SSD_Single(0x57,0x69);
SSD_Single(0x58,0x0A);
SSD_Single(0x59,0x0A);	//VCL = -2.9V
SSD_Single(0x5A,0x28);	//VGH = 19V
SSD_Single(0x5B,0x19);	//VGL = -11V



//--- Gamma  ----//
SSD_Single(0x5D,0x7C);              
SSD_Single(0x5E,0x65);      
SSD_Single(0x5F,0x53);    
SSD_Single(0x60,0x48);    
SSD_Single(0x61,0x43);    
SSD_Single(0x62,0x35);    
SSD_Single(0x63,0x39);    
SSD_Single(0x64,0x23);    
SSD_Single(0x65,0x3D);    
SSD_Single(0x66,0x3C);    
SSD_Single(0x67,0x3D);    
SSD_Single(0x68,0x5A);    
SSD_Single(0x69,0x46);    
SSD_Single(0x6A,0x57);    
SSD_Single(0x6B,0x4B);    
SSD_Single(0x6C,0x49);    
SSD_Single(0x6D,0x2F);    
SSD_Single(0x6E,0x03);    
SSD_Single(0x6F,0x00);    
SSD_Single(0x70,0x7C);    
SSD_Single(0x71,0x65);    
SSD_Single(0x72,0x53);    
SSD_Single(0x73,0x48);    
SSD_Single(0x74,0x43);    
SSD_Single(0x75,0x35);    
SSD_Single(0x76,0x39);    
SSD_Single(0x77,0x23);    
SSD_Single(0x78,0x3D);    
SSD_Single(0x79,0x3C);    
SSD_Single(0x7A,0x3D);    
SSD_Single(0x7B,0x5A);    
SSD_Single(0x7C,0x46);    
SSD_Single(0x7D,0x57);    
SSD_Single(0x7E,0x4B);    
SSD_Single(0x7F,0x49);    
SSD_Single(0x80,0x2F);    
SSD_Single(0x81,0x03);    
SSD_Single(0x82,0x00);    


//Page2, for GIP
SSD_Single(0xE0,0x02);

//GIP_L Pin mapping
SSD_Single(0x00,0x47);
SSD_Single(0x01,0x47);  
SSD_Single(0x02,0x45);
SSD_Single(0x03,0x45);
SSD_Single(0x04,0x4B);
SSD_Single(0x05,0x4B);
SSD_Single(0x06,0x49);
SSD_Single(0x07,0x49);
SSD_Single(0x08,0x41);
SSD_Single(0x09,0x1F);
SSD_Single(0x0A,0x1F);
SSD_Single(0x0B,0x1F);
SSD_Single(0x0C,0x1F);
SSD_Single(0x0D,0x1F);
SSD_Single(0x0E,0x1F);
SSD_Single(0x0F,0x43);
SSD_Single(0x10,0x1F);
SSD_Single(0x11,0x1F);
SSD_Single(0x12,0x1F);
SSD_Single(0x13,0x1F);
SSD_Single(0x14,0x1F);
SSD_Single(0x15,0x1F);

//GIP_R Pin mapping
SSD_Single(0x16,0x46);
SSD_Single(0x17,0x46);
SSD_Single(0x18,0x44);
SSD_Single(0x19,0x44);
SSD_Single(0x1A,0x4A);
SSD_Single(0x1B,0x4A);
SSD_Single(0x1C,0x48);
SSD_Single(0x1D,0x48);
SSD_Single(0x1E,0x40);
SSD_Single(0x1F,0x1F);
SSD_Single(0x20,0x1F);
SSD_Single(0x21,0x1F);
SSD_Single(0x22,0x1F);
SSD_Single(0x23,0x1F);
SSD_Single(0x24,0x1F);
SSD_Single(0x25,0x42);
SSD_Single(0x26,0x1F);
SSD_Single(0x27,0x1F);
SSD_Single(0x28,0x1F);
SSD_Single(0x29,0x1F);
SSD_Single(0x2A,0x1F);
SSD_Single(0x2B,0x1F);
                      
//GIP_L_GS Pin mapping
SSD_Single(0x2C,0x11);
SSD_Single(0x2D,0x0F);   
SSD_Single(0x2E,0x0D); 
SSD_Single(0x2F,0x0B); 
SSD_Single(0x30,0x09); 
SSD_Single(0x31,0x07); 
SSD_Single(0x32,0x05); 
SSD_Single(0x33,0x18); 
SSD_Single(0x34,0x17); 
SSD_Single(0x35,0x1F); 
SSD_Single(0x36,0x01); 
SSD_Single(0x37,0x1F); 
SSD_Single(0x38,0x1F); 
SSD_Single(0x39,0x1F); 
SSD_Single(0x3A,0x1F); 
SSD_Single(0x3B,0x1F); 
SSD_Single(0x3C,0x1F); 
SSD_Single(0x3D,0x1F); 
SSD_Single(0x3E,0x1F); 
SSD_Single(0x3F,0x13); 
SSD_Single(0x40,0x1F); 
SSD_Single(0x41,0x1F);
 
//GIP_R_GS Pin mapping
SSD_Single(0x42,0x10);
SSD_Single(0x43,0x0E);   
SSD_Single(0x44,0x0C); 
SSD_Single(0x45,0x0A); 
SSD_Single(0x46,0x08); 
SSD_Single(0x47,0x06); 
SSD_Single(0x48,0x04); 
SSD_Single(0x49,0x18); 
SSD_Single(0x4A,0x17); 
SSD_Single(0x4B,0x1F); 
SSD_Single(0x4C,0x00); 
SSD_Single(0x4D,0x1F); 
SSD_Single(0x4E,0x1F); 
SSD_Single(0x4F,0x1F); 
SSD_Single(0x50,0x1F); 
SSD_Single(0x51,0x1F); 
SSD_Single(0x52,0x1F); 
SSD_Single(0x53,0x1F); 
SSD_Single(0x54,0x1F); 
SSD_Single(0x55,0x12); 
SSD_Single(0x56,0x1F); 
SSD_Single(0x57,0x1F); 

//GIP Timing  
SSD_Single(0x58,0x40);
SSD_Single(0x59,0x00);
SSD_Single(0x5A,0x00);
SSD_Single(0x5B,0x30);
SSD_Single(0x5C,0x03);
SSD_Single(0x5D,0x30);
SSD_Single(0x5E,0x01);
SSD_Single(0x5F,0x02);
SSD_Single(0x60,0x00);
SSD_Single(0x61,0x01);
SSD_Single(0x62,0x02);
SSD_Single(0x63,0x03);
SSD_Single(0x64,0x6B);
SSD_Single(0x65,0x00);
SSD_Single(0x66,0x00);
SSD_Single(0x67,0x73);
SSD_Single(0x68,0x05);
SSD_Single(0x69,0x06);
SSD_Single(0x6A,0x6B);
SSD_Single(0x6B,0x08);
SSD_Single(0x6C,0x00);
SSD_Single(0x6D,0x04);
SSD_Single(0x6E,0x04);
SSD_Single(0x6F,0x88);
SSD_Single(0x70,0x00);
SSD_Single(0x71,0x00);
SSD_Single(0x72,0x06);
SSD_Single(0x73,0x7B);
SSD_Single(0x74,0x00);
SSD_Single(0x75,0x07);
SSD_Single(0x76,0x00);
SSD_Single(0x77,0x5D);
SSD_Single(0x78,0x17);
SSD_Single(0x79,0x1F);
SSD_Single(0x7A,0x00);
SSD_Single(0x7B,0x00);
SSD_Single(0x7C,0x00);
SSD_Single(0x7D,0x03);
SSD_Single(0x7E,0x7B);


//Page1
SSD_Single(0xE0,0x01);
SSD_Single(0x0E,0x01);	//LEDON output VCSW2


//Page3
SSD_Single(0xE0,0x03);
SSD_Single(0x98,0x2F);	//From 2E to 2F, LED_VOL

//Page4
SSD_Single(0xE0,0x04);
SSD_Single(0x09,0x10);
SSD_Single(0x2B,0x2B);
SSD_Single(0x2E,0x44);

//Page0
SSD_Single(0xE0,0x00);
SSD_Single(0xE6,0x02);
SSD_Single(0xE7,0x02);


#if 0
dsi_ctl &= ~1;
usleep(10000);

dsi_write(REG_DSI_CTL, dsi_ctl); /* disable DSI clock */
dsi_delay();

dsi_ctl |= 1;
usleep(10000);

dsi_write(REG_DSI_CTL, dsi_ctl); /* enable DSI clock */
dsi_delay();
#endif
#endif

//for(;;)
{

        printf(".");
        dsi_send_lp_short(0x15, 0x11, 0x00); /* send DCS SLEEP_OUT */
        delay(panel->cmd_delay);

        dsi_send_lp_short(0x15, 0x29, 0x00); /* send DCS DISPLAY_ON */
        delay(panel->cmd_delay);

        dsi_send_lp_short(0x15, 0x38, 0x00); /* send DCS EXIT_IDLE_MODE */
        delay(panel->cmd_delay);

        //dsi_send_lp_short(0x15, 0x21, 0x00); /* send DCS ENTER_INVERT_MODE */
        //delay(panel->cmd_delay);
}

    dsi_write(REG_H_FRONT_PORCH, panel->h_front_porch);
    dsi_write(REG_H_BACK_PORCH, panel->h_back_porch);
    dsi_write(REG_H_ACTIVE, panel->width * 3);
    dsi_write(REG_H_TOTAL, panel->frame_gap);
    dsi_write(REG_V_BACK_PORCH, panel->v_back_porch);
    dsi_write(REG_V_FRONT_PORCH, panel->v_back_porch + panel->height);
    dsi_write(REG_V_ACTIVE,
              panel->v_back_porch + panel->height + panel->v_front_porch);
    dsi_write(REG_V_TOTAL,
              panel->v_back_porch + panel->height + panel->v_front_porch);

    dsi_write(REG_TIMING_CTL, 1); /* start display refresh */
}

void dsi_force_lp(int force)
{
    dsi_write(REG_TIMING_CTL, 1 | (force ? 2 : 0));
}

int dsi_calc_vrefresh(struct dsi_panel_config *panel)
{
    int vtotal = panel->height + panel->v_front_porch + panel->v_back_porch;
    int htotal = panel->h_front_porch + panel->width * 3 + panel->h_back_porch +
                 6 + 6 + 6 + 4;

    //int phy_freq = pll_phy_freq();

    //if (panel->pll_cfg)
    //{
      //  phy_freq = BASE_CLOCK * panel->pll_cfg->mul / panel->pll_cfg->phy_div;
    //}
    int phy_freq = 500000000;
    int byte_clock = phy_freq / 8 * panel->num_lanes;
    return byte_clock / (htotal * vtotal);
}

int dsi_calc_bitrate(struct dsi_panel_config *panel)
{
    int phy_freq = 500000000; //pll_phy_freq();

    if (panel->pll_cfg)
    {
        phy_freq = 500000000;// BASE_CLOCK * panel->pll_cfg->mul / panel->pll_cfg->phy_div;
    }

    return phy_freq / 1000000;
}
