/*
agsm_kickstart_lbr.ino v 0.92/20150313 - a-gsm 2.064 KICKSTART LIBRARY SUPPORT
COPYRIGHT (c) 2015 Dragos Iosub / R&D Software Solutions srl

You are legaly entitled to use this SOFTWARE ONLY IN CONJUNCTION WITH a-gsm DEVICES USAGE. Modifications, derivates and redistribution 
of this software must include unmodified this COPYRIGHT NOTICE. You can redistribute this SOFTWARE and/or modify it under the terms 
of this COPYRIGHT NOTICE. Any other usage may be permited only after written notice of Dragos Iosub / R&D Software Solutions srl.

This SOFTWARE is distributed is provide "AS IS" in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Dragos Iosub, Bucharest 2015.
http://itbrainpower.net
***************************************************************************************
SOFTWARE:
This file MUST be present, toghether with "kickstart.ino", inside a folder named 
"kickstart"!
ARDUINO environment can be started clicking on "kickstart.ino".
***************************************************************************************
HARDWARE:
Read the readme file(s) inside the arhive/folder.
***************************************************************************************
HEALTH AND SAFETY WARNING!!!!!!!!!!!!!!!!!!!!
High power audio (around 700mW RMS)! You can damage your years! 
Use it with care when headset is connected. USE IT AT YOUR OWN RISK!
We recomend to set the volume at maximum level 20!
***************************************************************************************
*/
/*
In order to make your Arduino serial communication reliable (especially for Arduino Uno) with a-gsm shield, you must edit: 
C:\Program Files\Arduino\libraries\SoftwareSerial\SoftwareSerial.h 

comment the line 42 
#define _SS_MAX_RX_BUFF 64 

this, will look after that like: 
//#define _SS_MAX_RX_BUFF 64

and add bellow the next line: 
  
#define _SS_MAX_RX_BUFF 128

You just increased the RX buffer size for UNO and other "snails". 

Now you can compile and upload your code supporting highier buffered serial input data.
*/


/*do not change under this line! Insteed, make one copy for playing with.*/


/*
Prepare a-gsm for audio part usage
High power audio (around 700mW RMS)! You can damage your years! Use it with care when headset is connected.
We recomend to use AT+CLVL=25, audio setup command in order to limit the output power.
*/


/*read GSM signal status*/
int getSignalStatus(){
	char i[2];
	int j=0;
	int h=0;//signal level
	int res = 0;
	char tmpChar[20];//40
	char* pch;
	clearBUFFD();		
	memset(tmpChar,0x00,sizeof(tmpChar));
	sprintf(i, ",\r");
	res = sendATcommand("AT+CSQ","OK","ERROR",10);
	if(res){
		pch = strtok (buffd," ");
		j=0;
		while (pch != NULL){//parsing received data
		 	if(j==1){
				memset(tmpChar,0x00,sizeof(tmpChar));
				sprintf(tmpChar,"RSSI :%s",pch);
				Serial.println(tmpChar);
				res= atoi(pch);
				if(res==99){
					h=0;
				} else if(res>-1&&res<8){//-113dBm -> -99dBm
					h=1;
				}else if(res>7&&res<13){//-99dBm -> -89dBm
					h=2;
				}else if(res>12&&res<18){//-89dBm -> -79dBm
					h=3;
				}else if(res>17&&res<23){//-79dBm -> -69dBm
					h=4;
				}else if(res>22&&res<28){//-69dBm -> -59dBm
					h=5;
				}else if(res>27&&res<31){//-59dBm -> -53dBm
					h=6;
				}else if(res>=31){//>-53dBm 
					h=7;
				}

			}
		  	pch = strtok (NULL, i);
			j++;
		}
		clearBUFFD();

		//print signal as some graph
		Serial.print("Signal : ");
		j=0;
		while(j<h){
			Serial.print("#");delay(10);
			j++;
		}
		Serial.print("\r\n");
	}else Serial.println("SIGERR");
	clearBUFFD();
	return h;
}

/*clear buffd preparing it for writing*/
void clearBUFFD(void){//just clear the data buffer
    memset(buffd, 0x00, BUFFDSIZE);
}

/*send command -cmd to themodem, waiting Delay_mS msecs after that*/
size_t aGsmCMD(char* cmd, int Delay_mS){
    size_t retv;  
    retv = SSerial.println(cmd);
    delay(Delay_mS);
    return retv;
}

/*send string -str to the modem, NOT ADDING \r\n to the line end*/
size_t aGsmWRITE(char* str){
    size_t retv;  
    retv = SSerial.print(str);
    return retv;
}

/*return read char from the modem*/
//inline 
char aGsmREAD(void){
    char retv;
    retv = SSerial.read();
    return retv;
}

/*
Receive data from serial until event:
SUCCESS string - 1'st argument 
FAILURE string - second argument 
TIMEOUT - third argument in secconds!
return: int 
  -2 buffd BUFFER OVERFLOW (avoided)
  -1 TIMEOUT
  0  FAILURE
  1  SUCCESS
the string collected from modem => buffd
*/
int recUARTDATA( char* ok, char* err, int to){
	int res=0;
	char u8_c;
	int run = 1;
	int i=0;
	unsigned long startTime;
	clearBUFFD();
	startTime = millis();	
	//delay(10);
	while(run){
                t2.update();	
		if(strstr(buffd,ok)) {
			delay(200);
			#if defined(atDebug)
				Serial.println("ok");
			#endif
			clearSSerial();
			res=1;
			run=0;
		}
		else if(strstr(buffd,err)) {
			#if defined(atDebug)
				Serial.println("err");
			#endif
			run=0;
			clearSSerial();
			//clearBUFFD();
		}
		//delay(1);
		if(millis() - startTime > (unsigned long) to *1000) {
			#if defined(atDebug)
				Serial.println("to!");
			#endif
			//clearSSerial();
			run=0;
			res=-1;//timeout!
		}

		while(TXavailable()){
                        t2.update();
			u8_c = aGsmREAD();
                        if(i<BUFFDSIZE-1){ 
                        	buffd[i]=u8_c;
                        }else{
							#if defined(atDebug)
								Serial.println("bufover");
							#endif
                        	clearSSerial();
                          	return -2;//bufferoverflow
                        }
                        i++;
		}
	}
	return(res);
}
 
/*
send AT command, looking for SUCCES STRING(1'st) or FAILURE STRING(second), and TIMEOUT(third)
return 1 for succes, 2 for failure, -1 for timeout
modem response is loaded in buffd
*/ 
int sendATcommand(char* outstr, char* ok, char* err, int to){
	int res=0;
	clearSSerial();
	clearBUFFD();	
	#if defined(atDebug)
		Serial.println(outstr);
	#endif
        t2.update();
        aGsmCMD(outstr,1);
	res = recUARTDATA( ok,  err, to);
	return(res);
}

/*
  returns TRUE if chars are available in RX SERIAL_BUFFER (some chars has been received)
  check this function before call aGsmREAD()
*/
bool TXavailable(){
    int retv;
    retv = SSerial.available();
    return (retv > 0);
}

/*parseResponce("OK", "AT+GSN", tmpChar, "", 2);*/
int parseResponce(char* ok, char* head, char* retChar, char* separator , int index){
	char* pch0;
	char* pch1;
	//char tmpChar[180];
	//memset(tmpChar, 0x00, sizeof(tmpChar));
	memset(readBuffer, 0x00, sizeof(readBuffer));
	pch1 = strstr(buffd, ok);
	if (pch1 == 0) return -1;
	pch0 = strstr(buffd, head);
	pch0 = pch0+strlen(head);
	
	if(pch0[2] == 0x0A) 
		//strncpy(tmpChar, pch0+3, pch1 - pch0 - strlen(ok) - 5);
		strncpy(readBuffer, pch0+3, pch1 - pch0 - strlen(ok) - 5);
	else
		//strncpy(tmpChar, pch0, pch1 - pch0 - strlen(ok) - 2);
		strncpy(readBuffer, pch0, pch1 - pch0 - strlen(ok) - 2);

	//pch0 = strtok (tmpChar, separator);
	pch0 = strtok (readBuffer, separator);
	int j=0;
	while (pch0 != NULL){//parsing the message
		if(j <= index){
			if(j == index){
				sprintf(retChar,"%s",pch0);
				return 0;
			}
			j++;
		}
		pch0 = strtok (NULL, separator);
	}
	//sprintf(retChar,"%s",tmpChar);
	sprintf(retChar,"%s",readBuffer);
	return 0;
}

/*print modem registration status*/
int printRegistration(int cmdValue){
	if(cmdValue==1) Serial.println("Registrated (home)");
	else if(cmdValue==5) Serial.println("Registrated (roaming)");
	else Serial.println("Not registrated");
}

/*read modem registration status, 1 for GSM, 0 for GPRS*/
int registration(int type){
	char tmpChar[20];//40
	int res;
	if (type==1) res = sendATcommand("AT+CREG?","OK","ERROR",2); 
	else res = sendATcommand("AT+CGREG?","OK","ERROR",2); 
	if(res==1){
		memset(tmpChar,0x00, sizeof(tmpChar));
		if (type==1) res = parseResponce("OK", "+CREG:", tmpChar, ",", 1);
		else res = parseResponce("OK", "+CGREG:", tmpChar, ",", 1);
		printRegistration(atoi(tmpChar));
	}else Serial.print("CMD timeout/error");
}

/*prepare/check modem is ready for SMS handling*/
void setupMODEMforSMSusage(){
	if(ready4SMS > 0) return;
	int res;
	res = 0; 
	while (res!=1){//wait 
		res = sendATcommand("AT+CPBS?","OK","ERROR",5);
		//delay(500);
	}
	//set SIMM memory as active
	sendATcommand("AT+CPBS=\"SM\"","OK","ERROR",7);
	//is MODEM ready for SMS?
	res = 0; 
	while (res!=1){//wait 
		res = sendATcommand("AT+CPBS?","OK","ERROR",5);
		//delay(500);
	}				
	//set SMS mode TEXT		
	res = sendATcommand("AT+CMGF=1","OK\r\n","ERROR\r\n",5);
	res = sendATcommand("AT+CNMI=2,0,0,0,0","OK\r\n","ERROR\r\n",5);
	ready4SMS = 1;
	//delay(500);
	clearSSerial();     
}

/*send "message" to "phno". Returns 1 for success, 0 for failure*/
int sendSMS(char* phno, char* message){
	if(ready4SMS != 1)	
		setupMODEMforSMSusage();
	int res=0;
	clearBUFFD();
	sprintf(buffd,"AT+CMGS=\"%s\",129\r", phno);
	SSerial.print(buffd);
	res = recUARTDATA(">","ERROR",12);  
	if(res==1) {    
		clearBUFFD();
		sprintf(message,"%s%c",message,0x1A);
		res = sendATcommand(message, "OK","ERROR",30);  
		//delay(150);
		//if(res==1 && strstr(buffd,"+CMGS:")) Serial.println("SMS succeed");
		//else Serial.println("SMS error"); 
		//delay(150);
	}
	return res;

}

/*just read one line(looks for LF char) and return the line loaded into buffd*/
void readline(){/*todo some timer*/
	int cnt=0;
	char c;
	while(1){
		if(TXavailable()){
			c=SSerial.read();
			if(c == '\n') break;
			buffd[cnt++] = c;
		}
	}
	buffd[cnt++] = 0x00;//add string terminator
}
/*read SMS from SMSindex location*/
void readSMS(int SMSindex){        
	clearBUFFD();
	if(ready4SMS != 1) setupMODEMforSMSusage();
	if(totSMS<1) listSMS();
	if(SMSindex > noSMS || SMSindex < 1) return;
	int cnt=0;
	int j=0;
	int run=1;
	char c;
	unsigned long startTime = 0;
	clearSSerial();
	SSerial.println();//send command to modem

        String tmp="AT+CMGR_=";
        tmp="AT+CMGR=";
        tmp=tmp+SMSindex;
               tmp=tmp+",0";
	//SSerial.println("");//send command to modem
	//SSerial.print("AT+CMGR=");//send command to modem
	//SSerial.print(SMSindex);//send command to modem
	//SSerial.println(",0");//send command to modem
	SSerial.println(tmp);//send command to modem

	delay(1);
	readline();//just 2 remove modem cmd echo
	//readline();//just 2 remove second line ==> (containing OK / +CMS ERROR / SMS header)
	if(strlen(buffd)<20)
		  if(strstr(buffd,"OK")||strstr(buffd,"ERROR:")) {//check empty message location or beyond interval SMS location
					  clearBUFFD();
					  clearSSerial();
					  return;
		  }
	startTime = millis();
	while(run){//here
	  if(TXavailable()){
		  c = SSerial.read();
		  if(c == '\n') {//look for CR char(end of SMS message)
			  buffd[cnt-1]=0x2c;
			  break;
		  }
		  if(cnt < BUFFDSIZE-1) {
			  buffd[cnt] = c;
		  } else {//buffer overflow here (avoided)
			  break;
		  }
		  cnt++;
	  }
	  else if(millis() - startTime > (unsigned long) 2000) //unblocking procedure 
			  break;
	  
	}
	startTime = millis();
	while(run){//here
	  if(TXavailable()){
		  c = SSerial.read();
		  if(c == '\r') {//look for CR char(end of SMS message)
			  buffd[cnt]=0x00;
			  break;
		  }
		  if(cnt < BUFFDSIZE-1) {
			  buffd[cnt] = c;
		  } else {//buffer overflow here (avoided)
			  break;
		  }
		  cnt++;
	  }
	  else if(millis() - startTime > (unsigned long) 2000) //unblocking procedure 
			  break;
	  
	}


	buffd[cnt]=0x00;//add string terminator
	clearSSerial();//clear remaining serial chars
}

/*print all SMSs*/
void readAllSMS(){
	listSMS();
	int cnt;
	cnt = noSMS;
	while (cnt>0){
		Serial.print("SMS ");//noSMS
		Serial.println(cnt);
		readSMS(cnt);
		Serial.println(buffd);delay(50);
		clearBUFFD();
		clearSSerial();
		cnt--;
	}
}

/*delete SMS from SMSindex location...to make space, delete from big to small SMSindex*/
void deleteSMS(int SMSindex){
	if(ready4SMS != 1)	
		setupMODEMforSMSusage();
	char tmpChar[20];//40
	memset(tmpChar,0x00, sizeof(tmpChar));
	clearBUFFD();     
	sprintf(tmpChar,"AT+CMGD=%i\r",SMSindex);//format the delete command 
	sendATcommand(tmpChar,"OK","ERROR",3);//send command to modem
}

/*list SMS, total SMS locations(capacity) ==> noSMS, last used SMS location ==> noSMS*/
void listSMS(){
	if(ready4SMS != 1)	
		setupMODEMforSMSusage();
	int res=0;
	int j=0;
	char * pch;
	while(res!=1){
	res = sendATcommand("AT+CPMS?","OK","+CMS ERROR:",10);//+CPMS: "SM",8,50,"SM",8,50,"SM",8,50// +CMS ERROR:
}
        noSMS=getValue(buffd, ',', 1).toInt();
 	totSMS=getValue(buffd, ',', 2).toInt();

}
/*just restart the modem*/
void restartMODEM(){
	clearSSerial();
	delay(100); 
	if(digitalRead(statusPIN)){
		Serial.println("powerOFF"); delay(500);
		digitalWrite(powerPIN, HIGH);    
		delay(1000);                  
		digitalWrite(powerPIN, LOW);  
		powerState=0;
		delay(8000);
	}

	if(!digitalRead(statusPIN)){ // not running
		clearSSerial();delay(100);
		Serial.println("try restart"); delay(500);
		digitalWrite(powerPIN, HIGH);    
		delay(1000);                  
		digitalWrite(powerPIN, LOW);   
		delay(8000);
		powerState = 1;
		state=1;
	}
}


/*just flush remaining chars from software serial (a-gsm)*/
void clearSSerial(){
	while(TXavailable()){
		ch = aGsmREAD();
	//	delay(0.5);
	} 
	//delay(100);
	ch=0x00;
} 

/*just flush remaining chars from serial (debug)*/
void clearSerial(){
	while(Serial.available()){
		ch = Serial.read();
		//delay(0.5);
	} 
	//delay(100);
	ch=0x00;
} 

String getValue(String data, char separator, int index)
{

    int maxIndex = data.length()-1;
    int j=0;
    String chunkVal = "";

    for(int i=0; i<=maxIndex && j<=index; i++)
    {
      chunkVal.concat(data[i]);

      if(data[i]==separator)
      {
        j++;

        if(j>index)
        {
          chunkVal.trim();
          return chunkVal;
        }    

        chunkVal = "";    
      }  
    }  
}
