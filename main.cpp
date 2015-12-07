#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <cstring>

#define MAX_ITEMS 50
#define MAX_COST 10000

using namespace std;

struct item {
	int weight, price, order;
};
struct result {
	bool presence[MAX_ITEMS];
	int price, weight;
};

int id, item_count, knapsack_size, items_price_total = 0, repeat = 1;
char display = 0, debug = 0;
vector<item> items;

void parseLine(string line) {
	int i = 0;
	if (debug) cout << "Parsing..." << endl;
	istringstream iss(line);
	iss >> id >> item_count >> knapsack_size;
	i = item_count;
	if (debug) cout << "Loaded " << id << ", " << item_count << ", " << knapsack_size << "." << endl;
	while (i > 0) {
		item itm;
		iss >> itm.weight >> itm.price;
		items_price_total += itm.price;
		itm.order = item_count - i;
		if (debug) cout << "\tLoaded values w " << itm.weight << ", p " << itm.price << "." << endl;
		items.push_back(itm);
		i--;
	}
	// loaded
}

result solveBruteforce(int item_no, bool status[MAX_ITEMS]) {
	if (item_no >= item_count) {
		// spocitej cenu a vrat
		result r;
		r.price = 0;
		r.weight = 0;
		for (int i = 0; i < item_count; i++) {
			r.presence[i] = status[i];
			if (status[i]) {
				r.price += items[i].price;
				r.weight += items[i].weight;
			}
		}
		if (r.weight <= knapsack_size) {
			return r;
		} else {
			r.price = -1;
			return r;
		}
	} else {
		status[item_no] = true;
		result r1 = solveBruteforce(item_no + 1, status);
		status[item_no] = false;
		result r2 = solveBruteforce(item_no + 1, status);
		if (r1.price > r2.price) {
			return r1;
		} else {
			return r2;
		}
	}
}

class CompareObjects {
	public:
		bool operator()(item p1, item p2) {
			if (p1.price/p1.weight > p2.price/p2.weight) {
				return true;
			} else {
				return false;
			}
		}
};

result solveHeuristics() {
	priority_queue<item, vector<item>, CompareObjects> pq(items.begin(), items.end()), pqtmp;
	result r, rtmp;
	r.price = 0;
	r.weight = 0;
	for (int i = 0; i < item_count; i++) {
		r.presence[i] = false;
	}
	for (int i = 0; i < item_count; i++) {
		rtmp.price = 0;
		rtmp.weight = 0;
		for (int j = 0; j < item_count; j++) {
			rtmp.presence[j] = false;
		}
		pqtmp = pq;
		for (int j = 0; j < i; j++)
			pqtmp.pop();
		while (pqtmp.size() > 0) {
			if (pqtmp.top().weight + rtmp.weight <= knapsack_size) {
				rtmp.weight += pqtmp.top().weight;
				rtmp.price += pqtmp.top().price;
				rtmp.presence[pqtmp.top().order] = true;
			}
			pqtmp.pop();
		}
		// eval tmp result, and replace r if beter
		if (rtmp.price > r.price) {
			r.price = rtmp.price;
			r.weight = rtmp.weight;
			for (int i = 0; i < item_count; i++) {
				r.presence[i] = rtmp.presence[i];
			}
		}
	}
	return r;
}

result solvePruning(int item_no, bool status[MAX_ITEMS], int upper, int &lower, int remaining_price, int price_sum) {
  // lower - spodni mez - cena nejlepsiho aktualne dosazeneho reseni
	// upper - vaha veci v batohu
	if (remaining_price + price_sum < lower) {
		result r;
		r.price = -1;
		return r;
	} else if (item_no >= item_count || upper >= knapsack_size) {
		// spocitej cenu a vrat
		result r;
		r.price = price_sum;
		if (price_sum > lower)
			lower = price_sum;
		r.weight = upper;
		for (int i = 0; i < item_count; i++) {
			r.presence[i] = status[i];
		}
		if (r.weight <= knapsack_size) {
			return r;
		} else {
			r.price = -1;
			return r;
		}
	} else {
		status[item_no] = true;
		result r1 = solvePruning(item_no + 1, status, upper + items[item_no].weight, lower, remaining_price - items[item_no].price, price_sum + items[item_no].price);
		status[item_no] = false;
		result r2 = solvePruning(item_no + 1, status, upper, lower, remaining_price, price_sum);
		if (r1.price > r2.price) {
			return r1;
		} else {
			return r2;
		}
	}
}

result knap_test() {
  int m[2][MAX_ITEMS][MAX_COST];
  int i, j, jj;
	result r;
	for (i = 0; i < item_count; i++) {
		r.presence[i] = false;
	}
	r.weight = 0;
  memset(m, 0, sizeof(int) * 2 * MAX_ITEMS * MAX_COST);
  for (i = 1; i <= item_count; i++) {
		for (j = 0; j <= knapsack_size; j++) {
			if (items[i-1].weight <= j) {
				if (m[0][i-1][j] > m[0][i-1][j-items[i-1].weight] + items[i-1].price) {
					m[0][i][j] = m[0][i-1][j];
					m[1][i][j] = m[1][i-1][j];
				} else {
					m[0][i][j] = m[0][i-1][j-items[i-1].weight] + items[i-1].price;
					m[1][i][j] = j-items[i-1].weight;
				}
			} else {
				m[0][i][j] = m[0][i-1][j];
				m[1][i][j] = m[1][i-1][j];
			}
		}
  }
	i = item_count;
	j = knapsack_size;
	while (i > 0) {
		jj = m[1][i][j];
		if (m[0][i][j] != m[0][i-1][jj]) {
			r.presence[i-1] = true;
			r.weight += items[i-1].weight;
		}
		i--;
		j = jj;
	}
	r.price = m[0][item_count][knapsack_size];
	return r;
}

map<pair<int,int>,result> knap_buff;

result knap2(int item_no, int weight) {
	map<pair<int,int>,result>::iterator it;
	if ((it = knap_buff.find(make_pair(item_no, weight))) != knap_buff.end())
		return it->second;
	if (item_no >= item_count || weight <= 0) {
		result r;
		r.price = 0;
		r.weight = 0;
		for (int i = 0; i < item_count; i++) {
			r.presence[i] = false;
		}
		knap_buff.insert(make_pair(make_pair(item_no, weight), r));
		return r;
	}
  result r1 = knap2(item_no + 1, weight - items[item_no].weight); // obsahuje predmet item_no
  result r2 = knap2(item_no + 1, weight); // neobsahuje predmet item_no
  if (r1.price + items[item_no].price > r2.price) {
		// pricteme cenu a oznacime predmet jako pouzity
		r1.presence[item_no] = true;
		r1.price += items[item_no].price;
		return r1;
	} else {
		// cenu nemenime a oznacime predmet jako nepouzity
		r2.presence[item_no] = false;
		return r2;
	}
}

result knap(int item_no, int weight) {
  knap2(item_no, weight);
  knap_buff.clear();
}

result fptas(double eps) {
  int m[2][MAX_ITEMS][MAX_COST];
  int i, j, jj;
	int price_max = 0, price_total = 0;
	result r;
	for (i = 0; i < item_count; i++) {
		r.presence[i] = false;
	}
	r.weight = 0;
	r.price = 0;
  //memset(m, MAX_COST, sizeof(int) * 2 * MAX_ITEMS * MAX_COST);
	// get max price
	for (int i = 0; i < item_count; i++) {
		if (price_max < items[i].price)
			price_max = items[i].price;
	}
	double k = (eps * price_max) / item_count;
	// save old prices, use dynamic knap, then restore back
	vector<int> old_prices;
	for (int i = 0; i < item_count; i++) {
		old_prices.push_back(items[i].price);
		items[i].price = (int)floor(items[i].price / k);
		price_total += items[i].price;
	}
	cout << "total=" << price_total << endl;
	for (j = 1; j <= price_total; j++) {
		m[0][0][j] = MAX_COST;
	}
	m[0][0][0] = 0;
  for (i = 1; i <= item_count; i++) {
		for (j = 0; j <= price_total; j++) {
			if (items[i-1].price <= j) {
				if (m[0][i-1][j] < m[0][i-1][j-items[i-1].price] + items[i-1].weight) { // pro vahy je obracene porovnani
					m[0][i][j] = m[0][i-1][j];
					m[1][i][j] = j;
				} else {
					m[0][i][j] = m[0][i-1][j-items[i-1].price] + items[i-1].weight;
					m[1][i][j] = j-items[i-1].price;
				}
			} else {
				m[0][i][j] = m[0][i-1][j];
				m[1][i][j] = j;
			}
		}
  }
	// finish, restore old prices and count them
	i = item_count;
	j = price_total;
	while (m[0][i][j] > knapsack_size && j > 0)
		j--;
	r.weight = m[0][i][j];
	while (i > 0) {
		jj = m[1][i][j];
		if (m[0][i][j] != m[0][i-1][jj]) {
			r.presence[i-1] = true;
			r.price += old_prices[i-1];
		}
		i--;
		j = jj;
	}
	for (i = 0; i < item_count; i++) {
		items[i].price = old_prices[i];
	}
	return r;
}

void printResult(int id, clock_t t1, clock_t t2, result r, string msg) {
	if (debug) {
		cout << msg << " solving " << id << " took " << 1000 * (t2 - t1) / CLOCKS_PER_SEC << "ms with result of " << r.price << " price and " << r.weight << " weight." << endl << "Resolved solution is ";
		for (int i = 0; i < item_count; i++)
			if (r.presence[i]) {
				cout << "1";
			} else {
				cout << "0";
				}
		cout << "." << endl;
	} else {
		cout << id << " " << msg << " time= " << 1000 * (t2 - t1) / CLOCKS_PER_SEC << " price= " << r.price << " weight= " << r.weight << " result= ";
		for (int i = 0; i < item_count; i++)
			if (r.presence[i]) {
				cout << "1";
			} else {
				cout << "0";
			}
		cout << endl;
	}
}

int main(int argc, char **argv) {
	result r;
	if (argc > 2) {
		if (argv[2][0] == 'X') {
			debug = 1;
		} else {
			display = argv[2][0];
			if (argc > 3) {
				repeat = atoi(argv[3]);
				cout << "Repeating is set to " << repeat << endl;
			}
			cout << "ID, time, price, weight, solution" << endl;
		}
	}
	if (debug) cout << "Opening file " << argv[1] << endl;
	ifstream f(argv[1]);
	if (f == NULL) {
		cerr << "Opening file failed!" << endl;
		return 1;
	}
	if (debug) cout << "Goint to parse" << endl;
	string line;
	while (getline(f, line)) {
		items.clear();
		clock_t t1 = clock();
		parseLine(line);
		clock_t t2 = clock();
		if (debug) {
			cout << "Loading line took " << 1000 * (t2 - t1) / CLOCKS_PER_SEC << "ms." << endl;
		}
		switch (display) {
			case 0:
			case 'b':
				t1 = clock();
				for (int i = 0; i < repeat; i++) {
					bool s[MAX_ITEMS] = {false};
					r = solveBruteforce(0,s);
				}
				t2 = clock();
				printResult(id, t1, t2, r, "Brute force"); // deterministic, so no need to find averages
				if (display != 0) break;
			case 'h':
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = solveHeuristics();
				t2 = clock();
				printResult(id, t1, t2, r, "Heuristics"); // also deterministic
				if (display != 0) break;
			case 'p':
				t1 = clock();
				for (int i = 0; i < repeat; i++) {
					bool s[MAX_ITEMS] = {false};
					int lwr = -1;
					r = solvePruning(0, s, 0, lwr, items_price_total, 0);
				}
				t2 = clock();
				printResult(id, t1, t2, r, "Pruning"); // also deterministic
				if (display != 0) break;
			case 'd':
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = knap(0, knapsack_size);
				t2 = clock();
				printResult(id, t1, t2, r, "Dynamic programming"); // also deterministic
				if (display != 0) break;
			case 'D':
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = knap_test();
				t2 = clock();
				printResult(id, t1, t2, r, "Dynamic programming"); // also deterministic
				if (display != 0) break;
			case 'f':
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.01);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.01"); // also deterministic
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.02);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.02"); // also deterministic
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.05);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.05"); // also deterministic
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.1);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.1"); // also deterministic
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.2);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.2"); // also deterministic
				t1 = clock();
				for (int i = 0; i < repeat; i++)
					r = fptas(0.5);
				t2 = clock();
				printResult(id, t1, t2, r, "FPTAS eps=0.5"); // also deterministic
				break;
			default:
				cout << "Invalid argument!" << endl;
		}
	}
	return 0;
}
