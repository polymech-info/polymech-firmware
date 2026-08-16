### Simple interface for *filtrex*, a bison and Javascript based expression parser


This is packaged as AMD module.


```js

     /1. Simple version, just evaluate one expression two times with different variables

     // Input from user (e.g. search filter)
     var expression = 'transactions <= 5 and abs(profit) > 20.5';

     var parser = new Expression();

     var aResult = parser.parse(expression,null,{
        variables:{
            transactions: 3,
            profit:-40.5
        }
    }); //returns 1

     var aResult = parser.parse(expression,null,{
        variables :{
            transactions: 3,
            profit:-14.5
        }
    }); //returns 1

     2. more advanced example, thing of a console input with real time feedback and autocompletion

     var expression = 'send{{Volume+4}} my garbage string {{transactions*2/Volume}}';

     var parser = new Expression();


     var aResult = parser.parse(expression, null, {
        variables: {
            Volume: 2,
            transactions: 3
        },
        delimiters: {
            begin: '{{',
            end: '}}'
        }
    });//returns 'send6 my garbage string 3' neat ?

```
