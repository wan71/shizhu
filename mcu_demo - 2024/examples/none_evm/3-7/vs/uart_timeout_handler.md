# uart_timeout_handler解析

## 何处使用uart_timeout_handler函数
```c
os_timer_init(&uart_at_timer, uart_timeout_handler,NULL);	//时间戳递增
```

开启uart_at_timer的地方为：
    在回调函数的	case AT_UART://这里处理 NB初始化时吐出的信息   里面

```c
   if(strncmp("+CTZE:",AT_uart_msg,5)==0)//时间戳
								{
									strmncpy(AT_uart_msg, 12, 19, timerev);
									co_printf("%s\r\n",timerev);
									convertToSixInt(timerev, &year, &month, &day, &hour, &minute, &second);
									
									timestart = getTimestamp(year, month, day, hour, minute, second);//发NB的
									
									timestart32 = getTimestamp32(year, month, day, hour, minute, second);//发蓝牙的
									
									os_timer_start(&uart_at_timer, 100, true);
									
//									os_timer_start(&printf_timer, 1000, true);

									co_printf("%x\r\n", timestart32);
									
									co_printf("%d\r\n", timestart32);
									
									mstime = timestart*10;
                  addElevenDigitNumbers(mstime, dt, timeresult);
									co_printf("%s\r\n", timeresult);
//									timetest = getTimestamp(2023, 1, 12, 22, 29, 31);
//									
// 								  co_printf("%x\r\n",timetest);
									
									
								}
```

1. 从NB突出的信息里面找 (strncmp("+CTZE:",AT_uart_msg,5)，timerev格式是2024/01/05,10:29:04
2. getTimestamp使用 convertToSixInt 函数来解析这个时间戳，然后使用 getTimestamp 和 getTimestamp32 函数将其转换为 64 位和 32 位的时间戳
3. mstime = timestart*10;对其中一个时间戳进行了乘以 10 的操作，并将其转换为一个 11 位的数字。
4. addElevenDigitNumbers解析

```c
void addElevenDigitNumbers(uint64_t a, uint64_t b, char* result) {
    uint64_t sum = a + b;
    sprintf(result, "%" PRIu64, sum);  //将计算得到的 sum 值按照 %" PRIu64 格式转换为字符串，并将结果存储在 result 字符数组中。
}

```
这个函数使用了 sprintf 函数，将计算得到的 sum 值按照 %" PRIu64 格式转换为字符串，并将结果存储在 result 字符数组中。这里使用了 PRIu64 宏，它是 C 语言标准库 <inttypes.h> 中定义的宏，用于指定 printf 和 sprintf 等格式化输出函数中输出 uint64_t 类型的值。



## 看看static void uart_timeout_handler(void *param)//时间戳递增
```c
static void uart_timeout_handler(void *param)//时间戳递增
{
//    uart_at(NULL);
	    dt++;  //uint64_t
	    cdt++;

			if(dt>60000)
				{
				dt=0;
			}
				
			if(cdt>60000)
				{
				cdt=0;
			}
				
			mstime = timestart*10;  //uint64_t
			
			addElevenDigitNumbers(mstime, dt, timeresult); //该函数主要目的是将两个 64 位的数字相加，并将结果转换为字符串，以便后续的处理或输出。
			
	    co_sprintf(timesend,"%x",timestart32);//蓝牙的
			//使用 co_sprintf 函数将 timestart32 转换为十六进制字符串，并将结果存储在 timesend 变量中。
}
```