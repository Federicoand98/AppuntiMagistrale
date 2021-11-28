package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAXPROC = 10
const MAX = 3 // capacità
const N int = 0
const S int = 1

var done = make(chan bool)
var termina = make(chan bool)

var entrata_grasso_N = make(chan int, MAXBUFF)
var entrata_grasso_S = make(chan int, MAXBUFF)
var entrata_magro_N = make(chan int, MAXBUFF)
var entrata_magro_S = make(chan int, MAXBUFF)
var uscita_grasso_N = make(chan int)
var uscita_grasso_S = make(chan int)
var uscita_magro_N = make(chan int)
var uscita_magro_S = make(chan int)
var ACK_G_N [MAXPROC]chan int
var ACK_G_S [MAXPROC]chan int
var ACK_M_N [MAXPROC]chan int
var ACK_M_S [MAXPROC]chan int

func grasso(id int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("Inizializzazione Grasso %d direzione %d in secondi %d\n", id, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)

	if dir == N {
		entrata_grasso_N <- id
		<-ACK_G_N[id]
		fmt.Printf("[Grasso %d] sul ponte in direzione NORD\n", id)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscita_grasso_N <- id
	} else if dir == S {
		entrata_grasso_S <- id
		<-ACK_G_S[id]
		fmt.Printf("[Grasso %d] sul ponte in direzione SUD\n", id)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscita_grasso_S <- id
	}
}

func magro(id int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("Inizializzazione Magro %d direzione %d in secondi %d\n", id, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)

	if dir == N {
		entrata_magro_N <- id
		<-ACK_M_N[id]
		fmt.Printf("[Magro %d] sul ponte in direzione NORD\n", id)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscita_magro_N <- id
	} else if dir == S {
		entrata_magro_S <- id
		<-ACK_M_S[id]
		fmt.Printf("[Magro %d] sul ponte in direzione SUD\n", id)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscita_magro_S <- id
	}
}

func server() {
	var cont_M_N int = 0
	var cont_M_S int = 0
	var cont_G_N int = 0
	var cont_G_S int = 0

	for {
		select {
		case x := <-when((cont_M_N+cont_G_N < MAX) && (cont_G_S == 0), entrata_magro_N):
			cont_M_N++
			fmt.Printf("[Ponte] entrato MAGRO %d in direzione Nord\n", x)
			ACK_M_N[x] <- 1
		case x := <-when((cont_M_S+cont_G_S < MAX) && (cont_G_N == 0), entrata_magro_S):
			cont_M_S++
			fmt.Printf("[Ponte] entrato MAGRO %d in direzione Sud\n", x)
			ACK_M_S[x] <- 1
		case x := <-when((cont_G_N+cont_M_N < MAX) && (cont_G_S == 0) && (cont_M_S == 0) && (len(entrata_magro_N) == 0), entrata_grasso_N):
			cont_G_N++
			fmt.Printf("[Ponte] entrato GRASSO %d in direzione Nord\n", x)
			ACK_G_N[x] <- 1
		case x := <-when((cont_G_S+cont_M_S < MAX) && (cont_G_N == 0) && (cont_M_N == 0) && (len(entrata_magro_S) == 0), entrata_grasso_S):
			cont_G_S++
			fmt.Printf("[Ponte] entrato GRASSO %d in direzione Sud\n", x)
			ACK_G_S[x] <- 1
		case x := <-uscita_magro_N:
			cont_M_N--
			fmt.Printf("[Ponte] uscito MAGRO %d in direzione Nord\n", x)
		case x := <-uscita_magro_S:
			cont_M_S--
			fmt.Printf("[Ponte] uscito MAGRO %d in direzione Sud\n", x)
		case x := <-uscita_grasso_N:
			cont_G_N--
			fmt.Printf("[Ponte] uscito GRASSO %d in direzione Nord\n", x)
		case x := <-uscita_grasso_S:
			cont_G_S--
			fmt.Printf("[Ponte] uscito GRASSO %d in direzione Sud\n", x)
		case <-termina:
			fmt.Printf("FINE...")
			done <- true
			return
		}
	}
}

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func main() {
	var MN int
	var MS int
	var GN int
	var GS int

	fmt.Printf("\n quanti GRASSI NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &GN)
	fmt.Printf("\n quanti GRASSI SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &GS)
	fmt.Printf("\n quanti MAGRI NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &MN)
	fmt.Printf("\n quanti MAGRI SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &MS)

	for i := 0; i < GN; i++ {
		ACK_G_N[i] = make(chan int, MAXBUFF)
	}

	for i := 0; i < GS; i++ {
		ACK_G_S[i] = make(chan int, MAXBUFF)
	}

	for i := 0; i < MN; i++ {
		ACK_M_N[i] = make(chan int, MAXBUFF)
	}

	for i := 0; i < MS; i++ {
		ACK_M_S[i] = make(chan int, MAXBUFF)
	}

	rand.Seed(time.Now().Unix())
	go server()

	for i := 0; i < GN; i++ {
		go grasso(i, N)
	}

	for i := 0; i < MN; i++ {
		go magro(i, N)
	}

	for i := 0; i < GS; i++ {
		go grasso(i, S)
	}

	for i := 0; i < MS; i++ {
		go magro(i, S)
	}

	for i := 0; i < MN+MS+GN+GS; i++ {
		<-done
	}

	termina <- true
	<-done

	fmt.Printf("FINE PROGRAMMA...")
}
