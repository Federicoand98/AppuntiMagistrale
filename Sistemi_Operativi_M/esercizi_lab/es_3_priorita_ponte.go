package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAXPROC = 100
const MAX = 3
const N int = 1
const S int = 0

var done = make(chan bool)
var termina = make(chan bool)
var entrataN = make(chan int, MAXBUFF)
var entrataS = make(chan int, MAXBUFF)
var uscitaN = make(chan int)
var uscitaS = make(chan int)
var ACK [MAXPROC]chan int

func veicolo(myid int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	time.Sleep(time.Duration(tt) * time.Second)

	if dir == N {
		entrataN <- myid
		<-ACK[myid]

		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)

		uscitaN <- myid
	} else {
		entrataS <- myid
		<-ACK[myid]

		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)

		uscitaS <- myid
	}

	done <- true
}

func server() {
	var contN int = 0
	var contS int = 0

	for {
		select {
		case x := <-when((contN < MAX) && (contS == 0), entrataN):
			contN++
			ACK[x] <- 1
		case x := <-when((contS < MAX) && (contN == 0) && (len(entrataN) == 0), entrataS):
			contS++
			ACK[x] <- 1
		case x := <-uscitaN:
			contN--
			fmt.Printf("[ponte]  uscito veicolo %d in direzione N!  \n", x)
		case x := <-uscitaS:
			contS--
			fmt.Printf("[ponte]  uscito veicolo %d in direzione S!  \n", x)
		case <-termina:
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
	var VN int
	var VS int

	fmt.Printf("\nquanti veicoli NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VN)
	fmt.Printf("\nquanti veicoli SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VS)

	for i := 0; i < VS; i++ {
		ACK[i] = make(chan int, MAXBUFF)
	}

	rand.Seed(time.Now().Unix())

	go server()

	for i := 0; i < VS; i++ {
		go veicolo(i, S)
	}

	for i := 0; i < VN; i++ {
		go veicolo(i, N)
	}

	for i := 0; i < VN+VS; i++ {
		<-done
	}

	termina <- true
	<-done

	fmt.Printf("\nFINITO")
}
