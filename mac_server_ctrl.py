#
# Mac server control test app. charles.park 2022/04/21
#
import sys
import os
import asyncio

from odroid_factory_api import API_MANAGER

API_SERVER_HOST = 'https://factory.hardkernel.kro.kr:8880'
API_TEST_SERVER_HOST = 'https://factory-dev.hardkernel.kro.kr:8880'

if __name__ == "__main__":
    args = sys.argv

    args_cnt = len(args)
    board_name = 'None'
    uuid = 'None'

    if args_cnt < 3:
        print ('usage : python3 mac_server_ctl.py [-r | -e] {board name} [-e:mac | -r:-f]')
        print ('      [option1] -r : request mac addr')
        print ('                -i : server info display')
        print ('                -e : erase mac addr [option2 {erase mac addr}]')
        print ('                -f : **warning** Controls the Factory Server')
        print ('      [option2] if option1 == -e')
        print ('                   Set erase mac address ')
        print ('                if option1 == -r')
        print ('                   Set control server to factory-server. default dev-server ')
        print ('      e.g) python3 mac_server_ctl.py -r n2l   : dev-server control for test')
        print ('           python3 mac_server_ctl.py -r n2l -f: factory-server control for product.')
        print ('           python3 mac_server_ctl.py -e n2l 001e06123456')
        quit()

    # odroid mac server control class get
    if args_cnt > 3:
        if args[3] == '-f' or args[4] == '-f':
            api_manager = API_MANAGER(board=args[2], server_host=API_SERVER_HOST)
        else :
            api_manager = API_MANAGER(board=args[2], server_host=API_TEST_SERVER_HOST)
    else :
        api_manager = API_MANAGER(board=args[2], server_host=API_TEST_SERVER_HOST)

    # Log message disaplay control (1 = enable)
    api_manager.log_en(0)

    # api infomation (server info) display
    api_manager.info_print()

    if (args_cnt == 3 or args_cnt == 4) and args[1] == '-r':
        # get mac address for board-name from the mac server.
        uuid = asyncio.run(api_manager.request_mac_addr())
        if "error" in uuid :
            print ('error, request_mac_addr()!\n', uuid)
            quit()
        else :
            # info update
            asyncio.run(api_manager.update_record({'uuid': uuid}))
    elif (args_cnt == 4 or args_cnt == 5) and args[1] == '-e':
        # remove current mac address info or set mac address

        if '001e06' in args[3] and len(args[3]) == 12:
            api_manager.clear(args[2])
            api_manager.set_mac_addr (args[3])
            msg = asyncio.run(api_manager.delete_assigned_sign())
        else:
            print ('error, input mac addr,', f'input mac = {args[3]}, mac len = {len(args[3])}')
            quit()

        if "error" in msg:
            print ('error, delete_assigned_sign()', msg)
        else :
            ret_msg = 'success' + ',' + 'delete_assigned_sign()'+ ','
            ret_msg = ret_msg + args[2] + ',' + args[3]
            print (ret_msg)
        quit()
    else :
        print ("error, unknown message", f'args[1] = {args[1]}, args[2] = {args[2]}')
        quit()

    # mac display
    if "001e06" in uuid:
        ret_msg = 'success' + ',' + 'request_mac_addr()' + ','
        ret_msg = ret_msg + api_manager.mac_addr + ',' + api_manager.uuid_mac
        print (ret_msg)
        # 할당 받은 주소를 제거한다. (테스트용시 사용. 사용 MAC이 계속 증가하는 것을 방지하기 위함.)
        if args_cnt < 4:
            asyncio.run(api_manager.delete_assigned_sign())
