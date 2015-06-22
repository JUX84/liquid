#pragma once

class Stats {
	private:
		static unsigned long peersCount;
	public:
		static void incPeersCount();
		static void decPeersCount();
		static unsigned long getPeersCount();
};
