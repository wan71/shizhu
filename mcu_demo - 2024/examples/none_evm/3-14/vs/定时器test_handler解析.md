# 智慧跳绳
## User_task_init函数分析
```csharp
//任务初始化函数
//因为硬件还没完全可以 所以测试功能时多用定时器来模拟跳绳
void user_task_init(void)
{	
    user_task_id = os_task_create(user_task_func);
	
    os_timer_init(&uart_at_timer,uart_timeout_handleNULL);	//时间戳递增
	os_timer_init(&read_timer, read_handler,NULL);	//读取输出
	os_timer_init(&test_timer, test_handler,NULL);	//模拟跳绳
	os_timer_start(&read_timer, 30, true);

}
```
### 1.test_handler函数-展示
```csharp
//测试用的中断 用定时器来定时增加跳绳数 模拟霍尔传感器 需要使用的话可以解开655行的定时器启动注释
static void test_handler(void *param)//跳绳
{
		jump.count++;
											
		jump.store[j].cindex=jump.count;
		jump.store[j].jump_dt=cdt;
											
	  j++;
	
	
	 if(j==20){
		 
		uint8_t TX_head[250]={0};
	  uint8_t TX_end[450]={0};
		
		int a=TX_NUM(jump.store);
										
		generateString(a,jump.store,TX_end);
		
    co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
		
		concatenateStrings(TX_head,TX_end,TX);		 

											
  	flash_write(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
//					co_printf("%s\r\n",TX);
//		flash_read(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
		 
		jump.package++;
	  jump.packageindex++;
											
		co_printf("%d\r\n",jump.count);
//		
//			co_printf("%s\r\n",TX);
		
//		flash_read(0x7D000,sizeof(TX),TX);
//					co_printf("%s\r\n",TX);
		 j=0;
	 }
	 if(jump.package>4)
	 {
		 	os_timer_stop(&test_timer);
	 }

}

```
### 函数解析
#### 一.jump结构体
```csharp
typedef struct {
	 char *deviceId;
	 int package;
	 int packageindex;
   char jump_time[21];  
	 int mode;
	 uint16_t count;	
	
   indexdata store[20];
	 
} JumpData;

```

```csharp
typedef struct {
	
   uint32_t jump_dt;
   uint16_t cindex;
} indexdata;

```
随着j++，
jump.store[j].cindex=jump.count;  //跳绳的个数
jump.store[j].jump_dt=cdt;  //跳绳的时间

用定时器的自加来模拟跳绳jump.count++;
并且存入结构体 store；
结构体store以20个为一组进行

#### 二.发送数据
1. 定义两个空的数组
```csharp
char TX_check[100]="";
char TX_read[100]="";
```
int a=TX_NUM(jump.store);原式为
```csharp
uint16_t TX_NUM(indexdata a[]){
	int n =0;

	while(n<20){
		if(a[n].cindex==0){
			break;
		}
		n++;
	}
	return n;
	
}
```
可见，a[n].cindex为跳绳的个数，TX_NUM判断确认跳绳个数不会为0。
如果为0就break，返回不为的部分，就是n

2. generateString(a,jump.store,TX_end)解析
```csharp
void  generateString(int a,indexdata b[],uint8_t TX[]) {
//    int length = (12 + 12) * a + 5; // 每组数据占据的长度为12个字符（包括逗号、冒号和分号），每个数字占据的长度为10个字符（包括负号）
    
    char str[450] ;

    
    strcpy(str, "{");
    int offset = 1; //offset 用于跟踪当前字符串的位置
    for (int i = 0; i < a; i++) {  
        offset += co_sprintf(str + offset, "index:%d,dt:%d;", b[i].cindex, b[i].jump_dt);
    }
    
    strcat(str, "}\"}\"\r\n");   //}}"\r\n

//strcat(str, "}\"}\"\r\n"); 将右花括号 }}、引号 "、大括号 } 和换行符 \r\n 添加到 str 末尾，以结束字符串。
    for (int i = 0; i < 450; i++) {
        TX[i] = (uint8_t)str[i]; //将 str 中的字符转换为对应的 uint8_t 类型
		} 
    
}
```
    
 offset += co_sprintf(str + offset, "index:%d,dt:%d;", b[i].cindex, b[i].jump_dt);
 这个代码的意思为for 循环遍历 b[] 数组中的元素，并使用 co_sprintf 将格式化的字符串拼接到 str 中
 最后,TX[]中的格式为
 {index:值,dt:值;index:值,dt:值;...}}

 3. 某种通信协议编码
   ```csharp
   co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
   ```
这个函数会将格式化的字符串写入到 TX_head 所指向的位置

所以这个格式的模版为
```csharp
"AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\""

```

    1. "AT+LCTM2MSEND=1,\"{"：命令或协议的标识开头部分。
    2.  \"serviceId\": 666：一个固定的字段 "serviceId" 并且其值为固定的整数 666。
    3. \"deviceId\":\"%s\"：表示设备ID的字段 "deviceId"，占位符 %s 将被实际的设备ID值替换。
    4. \"package\":\"%d\"：表示包编号的字段 "package"，占位符 %d 将被实际的包编号值替换。
    5. \"packageIndex\":\"%d\"：表示包索引的字段 "packageIndex"，占位符 %d 将被实际的包索引值替换。
    6. \"jtime\":\"%s\"：表示时间的字段 "jtime"，占位符 %s 将被实际的时间值替换。
    7. \"mode\":\"%d\"：表示模式的字段 "mode"，占位符 %d 将被实际的模式值替换。
    8. \"count\":\"%d\"：表示计数的字段 "count"，占位符 %d 将被实际的计数值替换。
    9. \"data\":\""：可能是数据字段的开头标识。

4. concatenateStrings(TX_head,TX_end,TX)解析
   将TX_head,TX_end复制到TX, TX格式为“uint8_t”

5. flash_write(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
   
   0x70000 + (jump.packageindex - 1) * sizeof(TX)：这是用于计算 Flash 存储器中的地址偏移量。0x70000 是 Flash 存储器的基地址。(jump.packageindex - 1) * sizeof(TX) 这部分计算了偏移量，其中 jump.packageindex 是包索引，用于计算特定数据应该存储的位置

6. 结尾
```csharp
	jump.package++;
	  jump.packageindex++;
											
		co_printf("%d\r\n",jump.count);
//		
//			co_printf("%s\r\n",TX);
		
//		flash_read(0x7D000,sizeof(TX),TX);
//					co_printf("%s\r\n",TX);
		 j=0;
```

```csharp
	 if(jump.package>4)
	 {
		 	os_timer_stop(&test_timer);
	 }
```
这个是用于测试的代码


### 哪里开启定时器
在回调函数，case AT_UART下，
```csharp
else if(strncmp("+LWM2MEVENT: 2",AT_uart_msg,14)==0){
		NB_work_flag=1;
		jump.deviceId="123";
        addElevenDigitNumbers(mstime, dt, jump.jump_time);
	    jump.mode=1;
         jump.package=1;
         jump.packageindex=1;
         jump.count=0;

   	    for (int k = 0; k < 20; k++) {
         jump.store[k].jump_dt = 0;
         jump.store[k].cindex = 0;
        }
		flash_erase(0x70000,0x2F000);
		os_timer_start(&test_timer, 150, true);
        co_printf("OK\r\n");
				          }				
```