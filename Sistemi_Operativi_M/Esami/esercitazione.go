package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
//Constants
/////////////////////////////////////////////////////////////////////
const MAXBUFF = 100
const MAXPROC = 10
const MAX = 5 // capacità

/////////////////////////////////////////////////////////////////////
//Structures
/////////////////////////////////////////////////////////////////////
type Richiesta struct {

}

/////////////////////////////////////////////////////////////////////
//Channels
/////////////////////////////////////////////////////////////////////
var entrata1 = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var entrata2 = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var uscita1 = make(chan int)
var uscita2 = make(chan int)
var ACK_T1 [MAXPROC]chan int 
var ACK_T2 [MAXPROC]chan int

/////////////////////////////////////////////////////////////////////
//GORoutine join
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Auxiliary functions
/////////////////////////////////////////////////////////////////////
func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

/////////////////////////////////////////////////////////////////////
//GORoutines
/////////////////////////////////////////////////////////////////////
func goroutine(myid int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1

     if dir == 1{
           fmt.Printf("Inizializzazione Thread tipo1 %d in secondi %d\n", myid, dir, tt)
        } else {
           fmt.Printf("Inizializzazione Thread tipo2 %d in secondi %d\n", myid, dir, tt)
        }
	time.Sleep(time.Duration(tt) * time.Second)


     if dir == 2{
            entrata1 <- myid // send asincrona
	    <-ACK_T1[myid] // attesa x sincronizzazione resto fermo fino a he il server non mi da conferma sul mio canale
            //fmt.Printf("")
	    tt = rand.Intn(5)
	    time.Sleep(time.Duration(tt) * time.Second)
	    uscita1 <- myid
     } else {
            entrata2 <- myid // send asincrona
	    <-ACK_T2[myid]    // attesa x sincronizzazione resto fermo fino a he il server non mi da conferma sul mio canale
            //fmt.Printf("")
	    tt = rand.Intn(10)
	    time.Sleep(time.Duration(tt) * time.Second)
	    uscita2 <- myid

     }

     done<-true

}


func server(){


var contT1 int = 0
var contT2 int = 0

	for {

		select {
                //thread tipo1
		case x := <-when( /*condizione da verificare*/, entrata1):
			contT1++
			fmt.Printf("Entrato Thread %d tipo1\n", x)
			ACK_T1[x] <- 1 // termine "call"

                //thread tipo2 
                case x := <-when( /*condizione da verificare*/, entrata1):
			contT2++
			fmt.Printf("Entrato Thread %d tipo2\n", x)
			ACK_T2[x] <- 1 // termine "call"

        
		case x := <-uscita1:
			contT1--
			fmt.Printf("Uscito Thread tipo 1 %d\n", x)

                
                case x := <-uscita2:
			contT2--
			fmt.Printf("Uscito Thread tipo 2 %d\n", x)

		case <-termina: // quando tutti i processi hanno finito
			fmt.Println("\n\n\nFINE!!!!!!")
			done <- true
			return
		}//select

	}//for


}//server






/////////////////////////////////////////////////////////////////////
//Test main
/////////////////////////////////////////////////////////////////////
func main() {
	var V1 int
	var V2 int
        

	fmt.Printf("\nQuanti Thread tipo1 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V1)
	fmt.Printf("\nQuanti Thread tipo2 (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &V2)
        
	//inizializzazione canali per le auto a nord e a sud 
	for i := 0; i < V1; i++ {
		ACK_T1[i] = make(chan int, MAXBUFF)
	}
        for i := 0; i < V2; i++ {
		ACK_T2[i] = make(chan int, MAXBUFF)
	}

	rand.Seed(time.Now().Unix())
	go server()

	for i := 0; i < T1; i++ {
		go goroutine(i,1)
	}
        for i := 0; i < T2; i++ {
		go goroutine(i,0)
	}

	for i := 0; i < V1+V2; i++ {
		<-done
	}
	
        termina <- true
	<-done
	fmt.Printf("\nHO FINITO!!! ^_- \n")
}


