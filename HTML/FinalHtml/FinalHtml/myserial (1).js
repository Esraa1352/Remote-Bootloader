// Copyright 2025 by Georgy Moshkin

let MYSERIAL =
{
	reader: null,
	writer: null,
	port: null,
	rxFifo: [],
	totalTimeout: 1000,
	charTimeout:  1,
	pollInterval:  1
}

async function comReadLoop()
{
	while (true)
	{
		
		if (MYSERIAL.port === null) return;
		
		let skipDelay = false;
		
		try
		{
			const { value, done } = await MYSERIAL.reader.read();
			
			if (done)
			{
				console.log("comReadLoop() exit");
				break;
			}
			
			if (value)
			{
				MYSERIAL.rxFifo.push(...value);
				skipDelay = true;
			}			
			
		}
		catch(error)
		{
			if (MYSERIAL.port === null) return;
			
			console.log("comReadLoop() error");
			await comClose();
			return;
		}
		
		if (skipDelay === false)
		{
			await new Promise(resolve => setTimeout(resolve, MYSERIAL.pollInterval));
		}

	}
}


async function comFlush()
{
	while (MYSERIAL.rxFifo.length>0)
	{
		await comRead(1000);
	}
}

async function comRead(needCnt, customTimeout = 0)
{
	if (customTimeout === 0)
	{
		customTimeout = MYSERIAL.totalTimeout + MYSERIAL.charTimeout*needCnt;
	}

	
	while ((MYSERIAL.rxFifo.length<needCnt) && (customTimeout>0))
	{
		await new Promise(resolve => setTimeout(resolve, MYSERIAL.pollInterval));
		customTimeout -= MYSERIAL.pollInterval;
	}

	if (MYSERIAL.rxFifo.length>=needCnt) 
	{
		let data = MYSERIAL.rxFifo.splice(0, needCnt);
		return data;
	}
	else
	{
		MYSERIAL.rxFifo = [];
		return [];
		
	}

}


async function comOpen(speed)
{
	if (MYSERIAL.port !== null) return false;
	
	let ports = await navigator.serial.getPorts();
	
	let tmpInfo = null;
	
	for (let i=0; i<ports.length; i++)
	{
		tmpInfo = ports[i].getInfo();
		
		if (tmpInfo !== null)
		{
			tmpPort = ports[i];
			break;
		}
	}
	
	if (tmpInfo === null)
	{
		try
		{
			tmpPort = await navigator.serial.requestPort();
		}
		catch (error)
		{
			return false;
		}
	}

	try
	{
		await tmpPort.open({ baudRate: speed });
		
		MYSERIAL.writer = tmpPort.writable.getWriter();
		MYSERIAL.reader = tmpPort.readable.getReader();
		MYSERIAL.port = tmpPort;
	}
	catch (error)
	{
		console.log('comOpen() error');
		return false;
	}
	
	comReadLoop();
	
	return true;

}

async function comClose()
{
	if (MYSERIAL.port === null) return false;
		
	try
	{
		let tmpPort = MYSERIAL.port;
		MYSERIAL.port = null;
		
		MYSERIAL.reader.releaseLock();
		MYSERIAL.writer.releaseLock();
		MYSERIAL.writer = null;
		MYSERIAL.reader = null
		
		await tmpPort.close();
		
		MYSERIAL.rxFifo = [];
		
		return true;		
	}
	catch (error)
	{
		console.log('comClose() error');
		return false;
	}
}	
		
	


async function comWrite(data)
{
	if (MYSERIAL.port === null) return false;
		
	try
	{	
		await MYSERIAL.writer.write(Uint8Array.from(data));
		return true;
	}
	catch (error)
	{
		console.log('comWrite() error');
		return false;
	}
}

function comTimeout(timeout, timeout2=1)
{
	MYSERIAL.totalTimeout = timeout;
	MYSERIAL.charTimeout = timeout2;
}





