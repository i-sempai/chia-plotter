/*
 * test_phase_1.cpp
 *
 *  Created on: May 25, 2021
 *      Author: mad
 */

#include <chia/entries.h>
#include <chia/phase1.hpp>
#include <chia/DiskSort.hpp>

#include <random>
#include <chrono>
#include <iostream>

int64_t get_wall_time_micros() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


int main(int argc, char** argv)
{
	const size_t num_threads = 4;
	const size_t log_num_buckets = argc > 1 ? atoi(argv[1]) : 9;
	
	uint8_t id[32] = {};
	for(int i = 0; i < sizeof(id); ++i) {
		id[i] = i + 1;
	}
	
	phase1::initialize();
	
	typedef DiskSort<phase1::entry_1, phase1::get_y<phase1::entry_1>> DiskSort1;
	typedef DiskSort<phase1::entry_2, phase1::get_y<phase1::entry_2>> DiskSort2;
	
	DiskSort1 sort_1(32 + kExtraBits, log_num_buckets, num_threads, "test.p1.t1");
	{
		Thread<std::vector<phase1::entry_1>> thread(
			[&sort_1](std::vector<phase1::entry_1>& input) {
				for(const auto& entry : input) {
					sort_1.add(entry);
//					std::cout << "x=" << entry.x << ", y=" << entry.y << std::endl;
				}
			}, "DiskSort/add");
		const auto begin = get_wall_time_micros();
		phase1::compute_f1(id, num_threads, &thread);
		sort_1.finish();
		std::cout << "F1 took " << (get_wall_time_micros() - begin) / 1e6 << " sec" << std::endl;
	}
	
	DiskSort2 sort_2(32 + kExtraBits, log_num_buckets, num_threads, "test.p1.t2");
	{
		const auto begin = get_wall_time_micros();
		const auto num_matches =
				phase1::compute_matches<phase1::entry_1, phase1::entry_2, phase1::tmp_entry_1>(
						2, num_threads, &sort_1, &sort_2, nullptr);
		std::cout << "T2 took " << (get_wall_time_micros() - begin) / 1e6 << " sec"
				<< ", found " << num_matches << " matches" << std::endl;
	}
	sort_1.close();
	
}


