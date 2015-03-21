# What is nxcc? #
## 1.	it's a tiny script compiler and virtual machine ##
## 2.	c-liked script with ‘class’ ##
## 3.	easy to be embeded ##
## 4.	also,it's not memory-safe and type-safe language... ##


---

# Hello world! #
```

#!/usr/local/bin/nxcc

api printf(fmt,...);

func main(argc,argv)
{
	printf("hello,world!\n");  ///classic ...
}

___eof___;///optional ...
```

---

# funny feature #

## 1.'::' is allowed inside identifier name ... ##
```
	var aaa::bbb;
	func TTT::do_sth(a,b,...){}
	const ccc::d123 = 0;
```

## 2.type-name is just a name for class-method/member accessing ... ##
> ### obj.method(args) ###
> this will cause a invoking of global function like following :
> ### $TYPE\_NAME\_OF(obj)::$method\_name(obj,args) ###

> Example:
```
	func AAA::do_sth(this,args){}
	
	var p : AAA;
	p.do_sth(args);  ///invoke AAA::do_sth(p,args);
```
## 3.array : just alloc stack/static memory ... ##
```
	var aaa1[10] ;      ///total size = 10 * sizeof(long)
	var aaa2[[10]] ;    ///total size = 10 * sizeof(char)
	var aaa3[[[10]]] ;  ///total size = 10 * sizeof(int)
```
## 4.no type casting , just ... ##
```
	var a : TYPE_A , b : TYPE_B , c : TYPE_C ;
	
	a = 0;
	a = b+1;
	c=a;
	b=b+1;
	a=b[0];
	c=b(1,2,3);
	
	a.xxx();    ///invoke TYPE_A::xxx(a) ...
	b.yyy(123); ///invoke TYPE_B::yyy(b,123)
```
## 5.no **operator , but we see pointer everywhere ... ##
```
	using struct/class to wrapper you code ...
```

---

# Another Example #
```
#!/usr/local/bin/nxcc

/**
 * demo code start here ...
 * 1.'TYPE_NAME' is just used to access method and global function only ...
 * 2.variable a just a pointer , a number ...
 * 3.'::' can stand inside a identifier 
 * 4.obj.method($args) will be translated to TYPE_NAME(obj)::method(obj,$args)
 * 5.no type-cast , every thing could be a pointer ...funny ...
 * 6.array , just alloc space only ...
 */

api printf(fmt,...); ///declare a extern service func 

///define a struct ...
class type_abc
{
    var id;                      ///4B
    var count : xxx_type;        ///4B
    var __buff[[16]];            ///16B
    var __obj_table[2]:yyy_type; ///2x4=8B
    func hello(){
    	printf("hello\n");
    }
    
    func echo(str)
    {
    	++this.count;
    	printf("count :%d content :%s\n",this.count,str);
    }
    
    func hack_add(count,...) ///demo only ...
    {
    	var ptr;
    	ptr = &count
    	
    	ptr[1] ; ///first args
    	ptr[2] ; ///seconf args
    	if(count==2)
    		return ptr[1]+ptr[2];
    	return 0;
    }
}

///just ...
func AAA::hello(me)
{
	printf("%d say hello...\n",me);
}

///show string only ...
func string::show(me)
{
	printf("%s\n",me);
}

///
/// entry point here
///
func main(argc,argv)
{
	var i=0,j;
	var p : AAA;
	
	for(i=0;i<1;++i)
	{
		"hello,world".show();  ///just hello world ...
	}
	
	p=123;
	p.hello();                 ///convinent call
	
	do_sth1().hack_add(1,2);   ///var-args
	
	var p1 : type_abc;
	
	p1.id = 0;
	p1.hello();
	
}

func do_sth(p1,p2)
{
}

var obj_ptr;

func do_sth1() : type_abc
{
	return obj_ptr;
}

___eof___; ///this means end of source , compiler will complete at this line..

just draw sth necessary here 
such as a comment? emmm~~~ 
```**


---


# Some details ... #
```
+----------+
|  Source  |
+----------+
     |
     v
+----------+
|   lexer  |
+----------+
     |
     | [Tokens...]
     v
+----------+
|  parser  |
+----------+
     |
     | [ASTs...]
     v
+------------+
| translator |
+------------+
     |
     |[Instructions...]
     v
+-----------+
|    vm     |
+-----------+
```

---

```
using 2 address instruction 

reg based vm ...

stack frame layout ...

|-->low-addr-->--                                 -->--high-addr-->|
|                                                                  |
+----+----+----+----+----+----+----+----+----+-------+-------------+
|varN|....|var2|var1|regN|....|reg2|reg1|ebp |retAddr|
+----+----+----+----+----+----+----+----+----+-------+-------------+ 
|                   |                   |
|<--variable-zone-->|<--register-zone-->|

1.each function has its own register-set (register-zone) (expression-register)
2.all expression-registers are mapped to local stack-frame ...
3.special register:
  eax : hold function result
  esp : stack top pointer
  ebp : stack base pointer
  eip : current instruction pointer

instruction structure:

+--------------------+
|      opcode        |
|--------------------|
|    reg1_index      |
|--------------------|
| immed / reg2_index |
+--------------------+

i just think that the aligned word will run faster than byte ..
so most filed type is int/long ...
fix me !!!
```

---


# script binary image layout #
```
+---------------------+
|    image-header     |
+---------------------+
|    text segment     |
+---------------------+
|    data segment     |
|---------------------|
| +----------------+  |
| |Export SYM Table|  |
| +----------------+  |
| |    name-ptr    |  |
| |----------------|  |
| |  Abs-Address   |  |
| +----------------+  |
| |    ... ...     |  |
| +----------------+  |
| +----------------+  |
| |Import SYM Table|  |
| +----------------+  |
| |    name-ptr    |  |
| |----------------|  |
| |   p_position   |  |
| +----------------+  |
| |    ... ...     |  |
| +----------------+  |
| +----------------+  |
| |RelocationTable |  |
| +----------------+  |
| |   RelocType    |  |
| |----------------|  |
| | p_RelocPosition|  |
| +----------------+  |
| |    ... ...     |  |
| +----------------+  |
+---------------------+

```