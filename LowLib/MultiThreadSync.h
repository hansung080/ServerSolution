#ifndef __LOWLIB_MULTITHREADSYNC_H__
#define __LOWLIB_MULTITHREADSYNC_H__

template <typename T>
class MultiThreadSync {
	friend class ThreadSync;

private:
	static CriticalSection mSync;

public:
	MultiThreadSync(VOID){}
	virtual ~MultiThreadSync(VOID){}

	class ThreadSync {
	public:
		ThreadSync(VOID){ T::mSync.enter(); }
		~ThreadSync(VOID){ T::mSync.leave(); }
	};
};

// static ��� ���� �ʱ�ȭ
// -> MultiThreadSync�� ��ü�� �������� �ʴ���, CriticalSection�� �����ڸ� ȣ���� �� �ֵ��� �Ѵ�.
template <typename T>
CriticalSection MultiThreadSync<T>::mSync;

#endif // __LOWLIB_MULTITHREADSYNC_H__