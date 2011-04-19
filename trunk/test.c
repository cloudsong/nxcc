#!/bin/nxcc
#define func
#define api
/**
 * this is a hello world demo ...
 */
///declare api here ...

//func kkk::def(this):kkk;


#define TTT

///declare api here ...
api printf(fmt,...);

func kkk::def(this)
{
	printf("this=%d\n",this);
}


func putd(i,d)
{
	printf("%d --> %d\n",i,d);

	//hahahahah();
}

func hello()
{
	printf("hello is called!!!\n");
	return 0xff;
}

var a000;

var def;
const xxxstr="1231aaaaaaaaaaaaa\n",xxxstr1="aaaaaaaaaaaa\n";

var my_str;


class string
{
	var buff[[16]] : string;
	var size : int;

	func init()
	{
		printf("init is called!!!\n");
	}

	func zero()
	{
		printf("zero is called");
	}
};

func main(argc,argv)
{
	//test1();
	//var aaa::bbb;

	//return type_test(1,2);

	//var kkk[[]]


	"hhhh".init();


	//aaa::bbb = "12312";

	my_str = xxxstr1;

	printf("%s\n",my_str);

	var p:kkk;

	p = 123;

	p.def();
	//return 0;

	var a1,b1;	
	def = 123;

	++def;

	def = hello;

	def();

	def = 100;
	printf("def=%d\n",def);
//	return 0;
	printf("hello()=%d\n",hello());
	
	var a0,b0;

//	a0 = & (a0 = b0);
//
//	b0 = &(a0==b0);
//	a0 =( b0,1);
	if(0)
	{

	}
	else if (1)
	{

	}
	else
	{

	}


	a0 = 0xFF;
	printf("a0 = %d\n",a0);

	//return 123;

	//aaa::bbb[0] = 'K';
	//printf("hello123 ==%s\n",aaa::bbb);

	//aaa::bbb.printf(1,2,3);

	var i;

	for (i=0;i<3;i = i+1)
	{
 		var j;

 		while(j<2)
 		{
 			printf("---\n");
 			break;
 			continue;
 		}
 
 		for(j=0;j<2;++j)
 		{
 			printf("-aa--\n");
 			break;
 		}
		putd(i,i);
		if(i>3)
		{
			break;
		}
		continue;
	}
	i = 0;
	do 
	{

	
		i = 0;
		do 
		{
			printf("do-while() ---test\n");
			++i;
			if(i>2) break;
			continue;
			///break;
		} while (i<3);

	++j;
	if(j>3) break; 
	continue;
	}
	while(0);
	a1 = 1;
	++a1;

	b1 = 2;
	++b1;

	a1=a1=2;

	printf("add-result = %d\n",a1+b1);

	var str[16];
	str[0]='a';
	str[[1]]='b';
	str[[2]]='c';
	str[3]='d';
	str[4]='\n';
	str[5]='\0';
//goto kkk1;
	printf(";;;;;str=%s   %c %c %c\n",str,str[[0]],str[[1]],str[[2]]);
//kkk1:
	//str.printf();

	//printf("%d\n",get_num1());

	var p1,p2;

	p1 = 0;

	p2 = &p1;

	p2[0] = 1;

	printf("----------p1=%d\n",p1);
	printf("p1=%d\n",p1);


	printf("argc=%d   argv=%s\n",argc,argv);

	echo("hahahaha",123123);
	return 0;
}

func echo(fmt,...)
{
	printf("fmt=%s arg1=%d\n",fmt,(&fmt)[1]);
	return 0;
}
