#pragma once


class Bakery
{
public:
	int n;
	// 스레드 최대 개수 8개 제한
	bool m_flag[8];
	int m_label[8];

	Bakery(int n) : n(n) {
		for (int i = 0; i < n; ++i)
		{
			m_flag[i] = false;
			m_label[i] = 0;
		}
	}

	Bakery() { }

	int Max()
	{
		int max = 0;
		for (int i = 0; i < n; ++i)
		{
			if (m_label[i] > max)	max = m_label[i];
		}
		return max;
	}

	void Lock(int thread_id)
	{
		int i = thread_id;
		m_flag[i] = false;
		m_label[i] = Max() + 1;
		for (int j = 0; j < n; j++)
		{
			if (j == i) continue;			// 자신 건너뛰기
			while (m_flag[j]) {}			// 다른 스레드가 번호 고르고 있으면 루프
			
			// 현재 루프 스레드 j가 더 작거나(우선순위가 높거나), 
			// 대기번호(label)가 같은면서 현재 루프 스레드의 id가 더 작으면(우선수위가 높으면) 대기
			while (m_label[j] != 0 && (m_label[j] < m_label[i] || (m_label[j] == m_label[i] && j < i)) ) {}	
		}
	}

	void UnLock(int thread_id)
	{
		m_flag[thread_id] = false;
	}
};
