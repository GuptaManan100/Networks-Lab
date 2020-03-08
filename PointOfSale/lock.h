#ifndef LOCK_MECH
#define LOCK_MECH

int globalLock = 0;

void acquireLock()
{
	while(globalLock>0)
	{
		usleep(10); 
	}
	globalLock++;
}

int releaseLock()
{
	globalLock--;
}

#endif