//------------------------------------------------------------------------------
/**
 * @file mac_server_ctrl.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID Mac Request server control application.
 * @version 0.1
 * @date 2022-12-09
 *
 * @package python3, 
 * @copyright Copyright (c) 2022
 *
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifndef	__MAC_SERVER_CTRL_H__
#define	__MAC_SERVER_CTRL_H__

//------------------------------------------------------------------------------
#define	MAC_SERVER_CTRL_DEV_SERVER			0
#define	MAC_SERVER_CTRL_FACTORY_SERVER		1

#define	MAC_SERVER_CTRL_TYPE_MAC			0
#define	MAC_SERVER_CTRL_TYPE_UUID			1
#define MAC_SERVER_CTRL_TYPE_UUID_SIZE		36
#define	MAC_SERVER_CTRL_TYPE_MAC_SIZE		12

//------------------------------------------------------------------------------
extern	int set_erase_mac 	(char *rboard, char *rmac, char ctrl_server);
extern  int get_mac_uuid    (char *rboard, char rtype, char *rdata, char ctrl_server);

#endif	//	#define	__MAC_SERVER_CTRL_H__
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
