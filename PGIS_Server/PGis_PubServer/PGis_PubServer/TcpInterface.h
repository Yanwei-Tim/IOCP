/*
**	FILENAME			tcpinterface.h
**
*/
#ifndef __TCPINTERFACE_H_
#define __TCPINTERFACE_H_

const int TCPSERVER_STARTSUCCESS = 0;
const int TCPSERVER_CLOSECLIENTSUCCESS = 0;

const int TCPCLIENT_CONNECTSUCCESS = 1;
const int TCPCLIENT_CREATERECVTHREADSUCCESS = 1;
const int TCPCLIENT_SENDSUCCESS = 1;

typedef enum enumTcpServerEvent
{
	TCPSERVER_EVENT_CLIENTJOIN,	   //�ͻ��˽���
	TCPSERVER_EVENT_CLIENTEXIT,	   //�ͻ����˳�
	TCPSERVER_EVENT_RECVDATA	   //�յ��ͻ�������	
}TCPSERVER_EVENT;

typedef enum enumTcpClientEvent
{
	TCPCLIENT_EVENT_RECVDATA = 0x0A,//���յ�����
	TCPCLIENT_EVENT_DISCONNECT//����������ӶϿ�
}TCPCLIENT_EVENT;

typedef enum enumTcpServerError
{
	TCPSERVER_ERROR_PORTEXCEEDRANGE = -1, //�˿ڳ�����Χ[0,65535]
	TCPSERVER_ERROR_NOSOCKDLL = -2,		  //�Ҳ���Sock��̬��,һ��Ϊ����ϵͳ����
	TCPSERVER_ERROR_NORIGHTSOCKDLL = -3,  //�Ҳ�����ȷ��Sock��̬��,һ��Ϊ����ϵͳ����
	TCPSERVER_ERROR_NONBLOCKING = -4,	  //���óɷ�����ģʽʧ��,һ��Ϊ����ϵͳ����	
	TCPSERVER_ERROR_CREATELISTNESOCK = -5,//����Listen Socketʱʧ��
	TCPSERVER_ERROR_BIND = -6,			  //bind �˿�ʧ��
	TCPSERVER_ERROR_LISTEN = -7,			  //����ʧ��
	TCPSERVER_ERROR_CREATEACCEPTTHREADFAIL = -8,	//Accept�̴߳���ʧ��
	TCPSERVER_ERROR_CREATERECVTHREADFAIL = -9,//recv �̴߳���ʧ��
	TCPSERVER_ERROR_NOTRIGHTSOCKID = -10,	//�����Sock ID���󣬳�����Χ���ID�����ÿͻ��Ѿ��˳�
	TCPSERVER_ERROR_SENDDATA = -11,			//��������ʱ����������ݴ�������ָ��Ϊ�ջ����ݳ���<=0	
	TCPSERVER_ERROR_SEND = -12,				//���ݷ��ʹ���
    TCPSERVER_INIT_ERROR = -13,
}TCPSERVER_ERROR;

typedef enum enumTcpClientError
{
	TCPCLIENT_ERROR_PORTEXCEEDRANGE = -1, //�˿ڳ�����Χ[0,65535]
	TCPCLIENT_ERROR_NOSOCKDLL = -2,		  //�Ҳ���Sock��̬��,һ��Ϊ����ϵͳ����
	TCPCLIENT_ERROR_NORIGHTSOCKDLL = -3,  //�Ҳ�����ȷ��Sock��̬��,һ��Ϊ����ϵͳ����
	TCPCLIENT_ERROR_NONBLOCKING = -4,	  //���óɷ�����ģʽʧ��,һ��Ϊ����ϵͳ����	
	TCPCLIENT_ERROR_CREATESOCK = -5,//����Socketʱʧ��
	TCPCLIENT_ERROR_CONNECT = -6,		//Connect ����ʱ����,������δ�������粻ͨ
	TCPCLIENT_ERROR_CREATERECVTHREADFAIL = -7,//���������߳�ʧ��
	TCPCLIENT_ERROR_BLOCKING = -8,//���ó�����ģʽʧ��,һ��Ϊ����ϵͳ����
	TCPCLIENT_ERROR_TIMEOUT = -9,//Connectʱ,��ʱ���ô��󣬷�ΧΪ(0,30]
	TCPCLIENT_ERROR_SENDDATA = -11,			//��������ʱ����������ݴ�������ָ��Ϊ�ջ����ݳ���<=0	
	TCPCLIENT_ERROR_SEND = -12,				//���ݷ��ʹ���
	TCPCLIENT_ERROR_NOTRIGHTSOCK = -13//���͵�Socket������,һ��Ϊδ�������ӷ�����
}TCPCLIENT_ERROR;

typedef struct tagTcpConnect
{
	char *pszRecvData;//ֻ�����¼�ΪTCPCLIENT_EVENT_RECVDATAʱ��Ч������ΪNULL
	int nRecvDataLen;//ֻ�����¼�ΪTCPCLIENT_EVENT_RECVDATAʱ��Ч������Ϊ0
}TcpConnect;

typedef struct tagTcpServerClient
{
	char szIP[32];//Ip��ַ����:192.168.21.122
    int nAppID;
	int nID;//�ͻ�ID�����пͻ��˽���ʱ�豣�����Ϣ���ڷ�������ʱʹ��
	int nPort;//�ͻ�ʹ�ö˿�
	char *pszRecvData;//ֻ�����¼�ΪTCPSERVER_EVENT_RECVDATAʱ��Ч������ΪNULL
	int nRecvDataLen;//ֻ�����¼�ΪTCPSERVER_EVENT_RECVDATAʱ��Ч������Ϊ0
}TcpServerClient;

//WANGZH_ALL
const int MAX_DATA_PACKAGE_LEN = 256;  //GIS���ط����İ�����󳤶�
const int MAIN_DATA_PACKAGE_LEN =  8;  //GIS���ط����İ�����С����

const int PACKAGE_HEAD_DATA_STX = 0x02; //0x02 ;  //��ͷ
const int PACKAGE_TAIL_DATA_ETX = 0x03; //0x03 ;  //��β
const int MAX_RECVICE_BUFFER = 1024;
const int PACKAGE_PUB_LEN = 6;

typedef	enum tagEnumProtocol
{
	NULL_PROTOCOL,
	UDP_PROTOCOL,
	FTP_PROTOCOL
}EnumProtocol;

typedef struct tagSocketInfo
{
	int nAppID;
	int nSocketID;
	int nPort;
	char szIP[32];
	int nRecvDataLen;
	char *pszRecvData;
}SocketInfo,*pSocketInfo;

typedef struct tagRecvClientData
{
	int             nAppID;
	int             nSockID;  //TCP����ID
	unsigned short	length;
	int				nCmdType;
	char MessagePacket[MAX_DATA_PACKAGE_LEN];
}RECV_CLIENTDATA;

#define WM_RECVICE_CLIENT_MESSAGE               WM_USER+SIMUL_USER_MESSAGE_BASE + 30

typedef enum enumAcmdType
{
	TYPE_LOGIN = 0x00,				
	TYPE_LOGIN_ACK = 0x00,

	TYPE_LOGINOUT = 0x01,
	TYPE_LOGINOUT_ACK = 0x01,

	TYPE_TICK = 0x02,		
	TYPE_TICK_ACK = 0x02,
}CMD_TYPE;

typedef enum enumSocketInfo
{
	TCP_CLIENT_DISCONNECT = 0x0F,
	TCP_CLIENT_CONNECTED,
	TCP_SERVER_DISCONNECT
}SOCKET_INFO;

typedef struct tagCMDTypeList
{
	UINT CMDID;
	char *HeadData;
	char *TailData;
}CMDTypeList;

#define SOCKET_PUBNETCLIENT_TYPEID      0
#define SOCKET_NEWCLIENT_TYPEDID		1 
#define SOCKET_OLDCLIENT_TYPEDID		2

static CMDTypeList Cmdtypelist[]={
	{SOCKET_PUBNETCLIENT_TYPEID,"*HITOM","#"},
	{SOCKET_NEWCLIENT_TYPEDID,"*HITOM","#"},
	{SOCKET_OLDCLIENT_TYPEDID,"*HYT,","#"},
};

///////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_RECVICE_BUFFER          /*1024+256*/4096
#define SOCKET_SERVER_PUBLICNET_ID     0
#define SOCKET_CLIENT_GISMAIN_ID       1


#define MAIN_PUBLIC_PACKAGE_LEN         4
#define MAX_PUBLIC_PACKAGE_LEN          256

#define MAX_PUBLICNET_DATE_TYPE         20

///////////////////////////////////////////////////////////////////////////////////////////

typedef enum enumTimerType
{
	TCP_CONN_FAIL,
	TCP_CONN_SUCCESS,
	TCP_WAIT_LOGIN_ACK,
	TCP_LOGIN_SUCCESS,
	TCP_WAIT_LOGIN_MSG, //tcp���ӳɹ���ȴ���GPRS���ӵ�¼��Ϣ��ʱ
	WAIT_CONN_TICK,
}TCPCONN_TYPE;

//GPS��������
typedef enum enumGpsDataType
{
	GPS_DATA_PRIVATENET_LINEOFF,//00��ʾר������GPS
	GPS_DATA_PRIVATENET_LINEON,//01��ʾר������GPS
	GPS_DATA_TINMEROUT,//02��ʾ�̶��ϴ�GPS
	GPS_DATA_MONITOR,//03��ʾ��ʱ����GPS
	GPS_DATA_BOUNDCALL,//04��ʾ�������GPS
	GPS_DATA_MY_POSITION,//05��ʾ����"�ҵ�λ��"��GPS
	GPS_DATA_MY_OFFDUTY,//06��ʾ�°�GPS
	GPS_DATA_MY_ONDUTY,//07��ʾ�ϰ�GPS
}GPS_DATA_TYPE;

//����Э�������ֶ���
typedef enum enumPubProtocalUp
{
	GPS_TINMEROUT_REPORT = 0x00,            //�̶�ʱ���������ϴ�GPS��Ϣ-00
	GPS_MONITOR_RESPONSE = 0x01,            //Ӧ��ʱ����GPSЭ��-01
	GPS_BOUNDCALL_RESPONSE = 0x02,          //Ӧ���������GPSЭ��-02
	SEND_MESSAGE_ANSWER = 0x03,             //������Ϣ�ظ���Ϣ

	TERMINAL_LOGIN_REPORT = 0x10,           //GPRS���ӵ�¼Э��-10
	TERMINAL_OPEN_REPORT = 0x11,            //������ϢЭ��-11 response
	GPS_TERMINAL_OPEN_REPORT = 0x12,        //����GPS��ϢЭ��-12
	TCPCONNECT_TICK_REPORT = 0x13,          //������Ϣ��ϢЭ��-13
	PRIVATENET_LINEON_REPORT = 0x14,        //ר������Э��-14
	PRIVATENET_LINEOFF_REPORT = 0x15,       //ר������Э��-15
	QUERY_MY_POSITION =0x16,                //�ϴ������ѯ�ҵ�λ����ϢЭ��-16
	QUERY_DESTINATION_POSITION = 0x17,      //�ϴ������ѯĿ�ĵؾ�γ��Э��-17
	CTL_TERMINAL_MOUNTGUARD_RESPONSE = 0x18,//Ӧ������ն��ϰ��°�-18
	GPS_HISTORY_REPORT = 0x19,              //��ʷGPS����

	CTL_PRIVATENET_LINE_RESPONSE = 0x20,    //Ӧ�����ר�����ߺ�ר����������-20
	CTL_TERMINAL_WAKEUP_RESPONSE = 0x21,    //Ӧ������ն˼�Ⱥң�к�ҡ��-21

	GPS_ALERTING_REPORT = 0x30,             //����Э��-30

	SET_COMMUNICATION_MODEL_RESPONSE = 0x60,//Ӧ������ͨѶ��ʽ����-60
	SET_GPRS_PARAMETER_RESPONSE = 0x61,		//Ӧ������GPRS����-61
	SET_GPS_PARAMETER_RESPONSE = 0x62,		//Ӧ������GSM����-62
	SET_TERMINAL_NUM_RESPONSE = 0x63,		//Ӧ�������ն�ר������-63
	SET_TEMP_NUM_RESPONSE = 0x64,           //Ӧ�����ö�̬��ʱ�������-64
	SET_ALERT_NUM_RESPONSE = 0x65,          //Ӧ������ר�����Ÿ澯����-65
	SET_SAFETYCHECK_NUM_RESPONSE = 0x66,    //Ӧ������ר���������-66
	SET_GPSCENTER_NUM_RESPONSE = 0x67,      //Ӧ������ר��GPS���ĺ���-67
	SET_BACK_MODEL_RESPONSE = 0x68,         //Ӧ������ר������ģʽ-68

	QUERY_COMMUNICATION_MODEL_RESPONSE=0x80,//Ӧ���ȡͨѶ��ʽ����-80
	QUERY_GPS_PARAMETER_RESPONSE = 0x81,	//Ӧ���ȡGPRS����-81
	QUERY_GSM_PARAMETER_RESPONSE = 0x82,	//Ӧ���ȡGSM����-82
	QUERY_TERMINAL_NUM_RESPONSE = 0x83,		//Ӧ���ȡ�ն�ר������-83
	QUERY_TEMP_NUM_RESPONSE = 0x84,         //Ӧ���ȡר����̬�������-84
	QUERY_ALERT_NUM_RESPONSE = 0x85,        //Ӧ���ȡר�����Ÿ澯����-85
	QUERY_SAFETYCHECK_NUM_RESPONSE = 0x86,  //Ӧ���ȡר���������-86
	QUERY_GPSCENTER_NUM_RESPONSE= 0x87,     //Ӧ���ȡר��GPS���ĺ���-87
	QUERY_BACK_MODEL_RESPONSE = 0x88,		//Ӧ���ȡר������ɨ��ģʽ-88
	QUERY_MAGNETIC_BOUND_RESPONSE = 0x89,	//Ӧ���ȡר����ǿ����-89
	QUERY_TERMINAL_TYPE_RESPONSE = 0x8A,    //Ӧ���ȡ�ն˻��Ͳ���-8A
	QUERY_WAKEUP_RESPONSE = 0x8B,           //Ӧ���ȡ�ն�ң��ҡ��״̬-8B

	QUERY_GPS_SEND_TINMEROUT = 0x8C,        //Ӧ���ȡ�ն˹̶��ϴ�GPS��Ϣʱ����-8C

	MAX_PROTOCOL_UP,

}PUBLIC_PROTOCAL_UP;

//����Э��
typedef enum  enumPubProtocalDown
{
	SET_GPS_TINMEROUT = 0,              //���ù̶��ϴ�GPSʱ��������-00
	SET_GPS_MONITOR = 0x01,             //�����·���ʱ����ָ��-01
	SET_GPS_BOUNDCALL = 0x02,           //���������Ϣ-02
	APP_SEND_MESSAGE = 0x03,            //�·�������Ϣ-03

	ACK_TERMINAL_LOGIN = 0x10,          //Ӧ���ն�GPRS��¼��Ϣ-10
	ACK_TERMINAL_OPEN = 0x11,           //Ӧ���ն˿�����Ϣ-11
	ACK_GPS_TERMINAL_OPEN = 0x12,       //Ӧ���ն˿���GPS��Ϣ-12
	ACK_TCPCONNECT_TICK = 0x13,         //Ӧ���ն�������Ϣ-13
	ACK_PRIVATENET_LINEON = 0x14,       //Ӧ��ר������-14
	ACK_PRIVATENET_LINEOFF = 0x15,      //Ӧ��ר������-15 
	ACK_TERMINAL_MOUNTGUARD = 0x18,     //Ӧ���ϡ��°ࡱ��Ϣ-18

	CTL_PRIVATENET_LINE = 0x20,         //����ר�����ߺ�ר����������-20
	CTL_TERMINAL_WAKEUP = 0x21,         //�����ն˼�Ⱥң�к�ҡ��-21

	ACK_GPS_ALERTING = 0x30,            //Ӧ�𱨾����������-30

	CFG_COMMUNICATION_MODEL_CMD = 0x60, //����ͨѶ��ʽ����-60
	CFG_GPRS_PARAMETER_CMD = 0x61,      //����GPRS����-61
	CFG_GSM_PARAMETER_CMD = 0x62,       //����GSM����-62
	CFG_TERMINAL_NUM_CMD = 0x63,        //�����ն�ר������-63
	CFG_TEMPGROUP_NUM_CMD = 0x64,       //���ö�̬��ʱ�������-64
	CFG_ALERT_NUM_CMD = 0x65,           //����ר�����Ÿ澯����-65
	CFG_SAFETYCHECK_NUM_CMD = 0x66,     //����ר���������-66
	CFG_GPSCENTER_NUM_CMD = 0x67,       //����ר��GPS���ĺ���-67
	CFG_BACK_MODEL_CMD = 0x68,          //����ר������ģʽ-68

	QUERY_COMMUNICATION_MODEL_CMD=0x80, //��ȡͨѶ��ʽ����-80
	QUERY_GPS_PARAMETER_CMD = 0x81,     //��ȡGPRS����-81
	QUERY_GSM_PARAMETER_CMD = 0x82,     //��ȡGSM����-82
	QUERY_TERMINAL_NUM_CMD = 0x83,      //��ȡ�ն�ר������-83
	QUERY_TEMPGROUP_NUM_CMD = 0x84,     //��ȡר����̬�������-84
	QUERY_ALERT_NUM_CMD = 0x85,         //��ȡר�����Ÿ澯����-85
	QUERY_SAFETYCHECK_NUM_CMD = 0x86,   //��ȡר���������-86
	QUERY_GPSCENTER_NUM_CMD= 0x87,      //��ȡר��GPS���ĺ���-87
	QUERY_BACK_MODEL_CMD = 0x88,        //��ȡר������ɨ��ģʽ-88
	QUERY_MAGNETIC_BOUND_CMD = 0x89,    //��ȡר����ǿ����-89
	QUERY_TERMINAL_TYPE_CMD = 0x8A,     //��ȡ�ն˻��Ͳ���-8A
	QUERY_WAKEUP_CMD = 0x8B,            //��ȡ�ն�ң��ҡ��״̬-8B
	QUERY_GPSSEND_INTERVAL_CMD = 0x8C,  //��ȡGPS�̶��ϴ�ʱ��������

	MAX_PUB_PROTOCAL_DOWN,

}PUBLIC_PROTOCAL_DOWN;

//�ն��ͺ�
typedef enum enumTermainalKind
{
	KIND_UNKNOWN = 0,
	KIND_TM800 = 1,
	KIND_TC890GM =2,
}TERMAINAL_KIND;

//����GPSģʽ
typedef enum enumOpenGpsModel
{
	OPEN_GPS_NOTCTRL, //������򲻿���
	OPEN_GPS_GPRSLOGIN,
	OPEN_GRPS_SOON, //GPS�����ϴ�
}OPEN_GPS_MODEL;

///////////////////////////////////////////////////////////////////////////////////////////
//
typedef enum enumGisAcmdType
{
	A_TYPE_LOGIN = 0x00,     //���ص�¼
	A_TYPE_LOGIN_ACK = 0x00, //��¼Ӧ��

	A_TYPE_TICK = 0x02,		//����Э��
	A_TYPE_TICK_ACK = 0x02, //����Ӧ��

	A_TYPE_TERMINAL_LINE = 0x03, //�ն˹�����GPRS/CDMA��������Э�� 1	GIS<-TER

	A_TYPE_QUERY_MY_LOCATION = 0x04, //�ն˲�ѯ�ҵ�λ��

	A_TYPE_QUERY_DESTINATION_POSITION = 0x05, //��ѯ����λ�þ�γ��

	//A_TYPE_SET_TEMPGROUP_ACK = 0x0A, //������ʱ����Ӧ�� 1		

	A_TYPE_APPCMD_TERMINAL = 0x0A, //�·�Ӧ��������ն�Э�� 1	GIS->TER

	A_TYPE_TERMINAL_GPS = 0x08,  //�ն��ϴ�GPSЭ�� 1			GIS<-TER

	// 	A_TYPE_CTRLCMD_TERMINAL = 0x0C, //�·���������Э��
	// 	A_TYPE_CTRLCMD_TERMINAL_ACK = 0x0C, //�ն��ϴ�Ӧ���������Э��
	// 
	// 	A_TYPE_TERMINAL_ALERT = 0x0D, //�ն˱���Э��
	// 	A_TYPE_TERMINAL_ALERT_ACK = 0x0D, //�·�Ӧ��ͽ���ն˱���Э��
	// 
	// 	A_TYPE_SETCMD_TERMINAL = 0x0E, //�·����ò������ն�Э�� 1	GIS->TER
	// 	A_TYPE_SETCMD_TERMINAL_ACK = 0x0E, //�ն�Ӧ���������Э��
	// 
	// 	A_TYPE_QUERYCFG_TERMINAL = 0x0F, //�·���ȡ�������ն�Э��
	// 	A_TYPE_QUERYCFG_TERMINAL_ACK = 0x0F, //�ն�Ӧ�������ȡЭ��
}GIS_A_CMD_TYPE; 

typedef enum enumCtrlAckType
{
	aCTRL_ACK_PRIVATENET_ONLINE, //��ʾӦ�����ר������
	aCTRL_ACK_PRIVATENET_OFFLINE,
	aCTRL_ACK_SLEEP, //��ʾӦ�����ר��ң��
	aCTRL_ACK_WAKEUP, //��ʾӦ�����ר��ң��
}CTRL_ACK_TYPE;

typedef enum enumGisMainSetCmd
{
	aSET_CMD_COMMUNICATION_MODEL = 0, //���õ�ǰͨѶ��ʽ��������
	aSET_CMD_GPRS_PARAMETER, //����GPRSͨѶ��������
	aSET_CMD_GSM_PARAMETER, //����GSMͨѶ����
	aSET_CMD_TERMINAL_NUM ,//�����ն�ר����������
	aSET_CMD_TEMPGROUP_NUM, //���ö�̬�������
	aSET_CMD_ALERT_NUM ,//����ר�����ű�����������
	aSET_CMD_SAFETYCHECK_NUM, //����ר�������������
	aSET_CMD_GPSCENTER_NUM, //����ר��GPS���ĺ�������
	aSET_CMD_BACK_MODEL ,//����ר������ɨ��ģʽ����
}
GIS_MAIN_SETCMD;
///////////////////////////////////////////////////////////////////////////////////////////
#endif