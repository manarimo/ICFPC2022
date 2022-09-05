#include <iostream>
#include <cmath>
using namespace std;

int main() {
   int n = 10;

   int mini = 999999;
   int miniA = 0;
   int miniB = 0;

   for (int A = 1; A <= n; A++) {
      for (int B = 1; B <= n; B++) {
         int cost = 0;
         for (int x = 0; x < A; x++) {
            for (int y = 1; y < n; y++) {
               cost += round(1.0 * n * n / y);
            }
         }
         for (int x = 1; x < A; x++) {
            cost += round(1.0 * n / x);
         }
         for (int y = 0; y < B; y++) {
            cost += round(7.0 * n * n / A / (n - y + 1));
         }
         for (int y = 0; y < B; y++) {
            for (int x = A; x < n; x++) {
               cost += round(1.0 * n * n / x);
            }
         }
         for (int y = 1; y < B; y++) {
            cost += round(1.0 * n / y);
         }
         
         if (B != n) {
            for (int x = A; x < n; x++) {
               cost += round(7.0 * n * n / B / (n - x + A));
            }
            cost += round(1.0 * n * n / A / max(n - B, B));
            for (int x = A; x < n; x++) {
               for (int y = B; y < n; y++) {
                  cost += round(1.0 * n * n / y);
               }
            }
            for (int x = A; x < n; x++) {
               cost += round(1.0 * n / x);
            }
         }
         if (cost < mini) {
            mini = cost;
            miniA = A;
            miniB = B;
         }
      }
   }
   cout << miniA << " " << miniB << " " << mini << endl;
}