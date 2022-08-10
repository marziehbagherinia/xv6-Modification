#include "types.h"
#include "stat.h"
#include "user.h"
// #include "stdio.h"

int gcd(int a, int b) 
{ 
	if (b == 0) 
		return a; 
	return gcd(b, a % b); 
} 

int findlcm(int arr[], int n) 
{ 
	int ans = arr[0]; 

	for (int i = 1; i < n; i++) 
		ans = (((arr[i] * ans)) / 
				(gcd(arr[i], ans))); 

	return ans; 
} 




int main(int argc, char *argv[]){
    int n = argc - 1;
    int arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    for(int i = 0; i < argc - 1; i++){
        arr[i] = atoi(argv[i + 1]);

    }
    int lcm = findlcm(arr, n);
    printf(1, "%d \n", lcm);

    // FILE * fp;
    // fp = fopen("./lcm_result.txt", "w");
    // fprint(fp, lcm);




    exit();

}

