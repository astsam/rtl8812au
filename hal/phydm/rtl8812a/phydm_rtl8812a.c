/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"

#include "../phydm_precomp.h"

#if (RTL8812A_SUPPORT == 1)

#if (defined(CONFIG_PATH_DIVERSITY))
void
odm_update_tx_path_8812a(struct dm_struct *dm, u8 path)
{
	struct _ODM_PATH_DIVERSITY_	*dm_path_div = &dm->dm_path_div;

	if (dm_path_div->resp_tx_path != path) {
		PHYDM_DBG(dm, DBG_PATH_DIV, "Need to Update Tx path\n");

		if (path == RF_PATH_A) {
			odm_set_bb_reg(dm, 0x80c, 0xFFF0, 0x111); /* Tx by Reg */
			odm_set_bb_reg(dm, 0x6d8, BIT(7) | BIT6, 1); /* Resp Tx by Txinfo */
		} else {
			odm_set_bb_reg(dm, 0x80c, 0xFFF0, 0x222); /* Tx by Reg */
			odm_set_bb_reg(dm, 0x6d8, BIT(7) | BIT6, 2); /* Resp Tx by Txinfo */
		}
	}
	dm_path_div->resp_tx_path = path;
	PHYDM_DBG(dm, DBG_PATH_DIV, "path=%s\n", (path == RF_PATH_A) ? "RF_PATH_A" : "RF_PATH_B");
}


void
odm_path_diversity_init_8812a(
	struct dm_struct	*dm
)
{
	u32	i;
	struct _ODM_PATH_DIVERSITY_	*dm_path_div = &dm->dm_path_div;

	odm_set_bb_reg(dm, 0x80c, BIT(29), 1); /* Tx path from Reg */
	odm_set_bb_reg(dm, 0x80c, 0xFFF0, 0x111); /* Tx by Reg */
	odm_set_bb_reg(dm, 0x6d8, BIT(7) | BIT6, 1); /* Resp Tx by Txinfo */
	odm_update_tx_path_8812a(dm, RF_PATH_A);

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		dm_path_div->path_sel[i] = 1; /* TxInfo default at path-A */
	}
}



void
odm_path_diversity_8812a(
	struct dm_struct	*dm
)
{
	u32	i, rssi_avg_a = 0, rssi_avg_b = 0, local_min_rssi, min_rssi = 0xFF;
	u8	tx_resp_path = 0, target_path;
	struct _ODM_PATH_DIVERSITY_	*dm_path_div = &dm->dm_path_div;
	struct cmn_sta_info	*entry;


	PHYDM_DBG(dm, DBG_PATH_DIV, "Odm_PathDiversity_8812A() =>\n");

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		entry = dm->phydm_sta_info[i];
		if (is_sta_active(entry)) {
			/* 2 Caculate RSSI per path */
			rssi_avg_a = (dm_path_div->path_a_cnt[i] != 0) ? (dm_path_div->path_a_sum[i] / dm_path_div->path_a_cnt[i]) : 0;
			rssi_avg_b = (dm_path_div->path_b_cnt[i] != 0) ? (dm_path_div->path_b_sum[i] / dm_path_div->path_b_cnt[i]) : 0;
			target_path = (rssi_avg_a == rssi_avg_b) ? dm_path_div->resp_tx_path : ((rssi_avg_a >= rssi_avg_b) ? RF_PATH_A : RF_PATH_B);
			PHYDM_DBG(dm, DBG_PATH_DIV, "mac_id=%d, path_a_sum=%d, path_a_cnt=%d\n", i, dm_path_div->path_a_sum[i], dm_path_div->path_a_cnt[i]);
			PHYDM_DBG(dm, DBG_PATH_DIV, "mac_id=%d, path_b_sum=%d, path_b_cnt=%d\n", i, dm_path_div->path_b_sum[i], dm_path_div->path_b_cnt[i]);
			PHYDM_DBG(dm, DBG_PATH_DIV, "mac_id=%d, rssi_avg_a= %d, rssi_avg_b= %d\n", i, rssi_avg_a, rssi_avg_b);

			/* 2 Select Resp Tx path */
			local_min_rssi = (rssi_avg_a > rssi_avg_b) ? rssi_avg_b : rssi_avg_a;
			if (local_min_rssi < min_rssi) {
				min_rssi = local_min_rssi;
				tx_resp_path = target_path;
			}

			/* 2 Select Tx DESC */
			if (target_path == RF_PATH_A)
				dm_path_div->path_sel[i] = 1;
			else
				dm_path_div->path_sel[i] = 2;

			PHYDM_DBG(dm, DBG_ANT_DIV, "Tx from TxInfo, target_path=%s\n",
				(target_path == RF_PATH_A) ? "RF_PATH_A" : "RF_PATH_B");
			PHYDM_DBG(dm, DBG_ANT_DIV, "dm_path_div->path_sel[%d] = %d\n", i, dm_path_div->path_sel[i]);

		}
		dm_path_div->path_a_cnt[i] = 0;
		dm_path_div->path_a_sum[i] = 0;
		dm_path_div->path_b_cnt[i] = 0;
		dm_path_div->path_b_sum[i] = 0;
	}

	/* 2 Update Tx path */
	odm_update_tx_path_8812a(dm, tx_resp_path);

}


#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
void
odm_set_tx_path_by_tx_info_8812a(
	struct dm_struct		*dm,
	u8			*desc,
	u8			mac_id
)
{
	struct _ODM_PATH_DIVERSITY_	*dm_path_div = &dm->dm_path_div;

	if ((dm->support_ic_type != ODM_RTL8812) || (!(dm->support_ability & ODM_BB_PATH_DIV)))
		return;

	SET_TX_DESC_TX_ANT_8812(desc, dm_path_div->path_sel[mac_id]);
}
#endif

#endif /* CONFIG_PATH_DIVERSITY */

#ifdef DYN_ANT_WEIGHTING_SUPPORT
void
phydm_dynamic_ant_weighting_8812a(
	void		*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	u8 rssi_l2h = 43, rssi_h2l = 37;
	u8 reg_8;

	if (dm->is_disable_dym_ant_weighting)
		return;

	if (*dm->channel <= 14) {
		if (dm->rssi_min >= rssi_l2h) {
			odm_set_bb_reg(dm, 0x854, BIT(3), 0x1);
			odm_set_bb_reg(dm, 0x850, BIT(21)|BIT(22), 0x0);

			/*equal weighting*/
			reg_8 = (u8)odm_get_bb_reg(dm, 0xf94, BIT(0)|BIT(1)|BIT(2));
			PHYDM_DBG(dm, ODM_COMP_API, "Equal weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n", dm->rssi_min, reg_8);
		} else if (dm->rssi_min <= rssi_h2l) {
			odm_set_bb_reg(dm, 0x854, BIT(3), 0x1);
			odm_set_bb_reg(dm, 0x850, BIT(21)|BIT(22), 0x1);
	
			/*fix sec_min_wgt = 1/2*/
			reg_8 = (u8)odm_get_bb_reg(dm, 0xf94, BIT(0)|BIT(1)|BIT(2));
			PHYDM_DBG(dm, ODM_COMP_API, "AGC weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n", dm->rssi_min, reg_8);
        }
	} else {
		odm_set_bb_reg(dm, 0x854, BIT(3), 0x1);
		odm_set_bb_reg(dm, 0x850, BIT(21)|BIT(22), 0x1);

			reg_8 = (u8)odm_get_bb_reg(dm, 0xf94, BIT(0)|BIT(1)|BIT(2));
			PHYDM_DBG(dm, ODM_COMP_API, "AGC weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n", dm->rssi_min, reg_8);
		/*fix sec_min_wgt = 1/2*/
    }

}
#endif

void
phydm_hwsetting_8812a(
	struct dm_struct		*dm
)
{
	phydm_dynamic_ant_weighting(dm);
}

#endif /* #if (RTL8812A_SUPPORT == 1) */
