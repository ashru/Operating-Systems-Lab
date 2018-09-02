#include <stdio.h>

int gcd(int x,int y) {
	return y ? gcd(y, x%y) : x;
}

int main(){
	int x, y;
	scanf("%d%d",&x,&y);
	printf("%d %d\n",gcd(x,y),x*y/gcd(x,y));
	return 0;
}
