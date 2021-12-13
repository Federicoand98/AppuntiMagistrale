package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBISCOTTI = 10
const M = 3
const TOTGELATI = 20

var done = make(chan bool)
var termina = make(chan bool)
var deposito_biscotto = make(chan int, 1)
var prelievo_biscotti = make(chan int, 1)
var rifornimento = make(chan int, 1)
var ack_rifornimento = make(chan int)
var ack_deposito = make(chan int)
var ack_prelievo = make(chan int)

func MB() {
	fmt.Printf("[MB]: partenza!\n")

	for i := 0; i < TOTGELATI*2; i++ {
		fmt.Printf("[MB]: produzione biscotto\n")
		time.Sleep(time.Duration(rand.Intn(2)+1) * time.Second)
		fmt.Printf("[MB]: deposito biscotto\n")
		deposito_biscotto <- 1
		<-ack_deposito
	}

	fmt.Printf("[MB]: finito\n")
	done <- true

	return
}

func MG() {
	fmt.Printf("[MG]: partenza!\n")

	var gelati_assemblabili int = 0

	for i := 0; i < TOTGELATI; i++ {
		fmt.Printf("[MG]: richiedo biscotti\n")
		prelievo_biscotti <- 2
		<-ack_prelievo

		if gelati_assemblabili == 0 {
			fmt.Printf("[MG]: richiedo rifornimento serbatoio\n")
			rifornimento <- 1
			gelati_assemblabili = <-ack_rifornimento
		}

		time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)
		gelati_assemblabili--

		fmt.Printf("[MG]: deposito gelato numero %d\n", i+1)
	}

	fmt.Printf("[MG]: finito\n")
	done <- true

	return
}

func Operaio() {
	for {
		select {
		case <-rifornimento:
			fmt.Printf("[Operaio]: richiesta di rifornimento\n")
			time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)
			ack_rifornimento <- M
		case <-termina:
			fmt.Printf("[Operaio]: termina\n")
			done <- true
			return
		}
	}
}

func Alimentatore() {
	var numero_biscotti int = 0

	for {
		fmt.Printf("[Alimentatore]: Numero biscotti = %d\n", numero_biscotti)

		select {
		case x := <-when(numero_biscotti < MAXBISCOTTI && (numero_biscotti < MAXBISCOTTI/2 || len(prelievo_biscotti) == 0), deposito_biscotto):
			fmt.Printf("[Alimentatore]: ricevuto biscotto\n")
			time.Sleep(time.Duration(rand.Intn(3)+1) * time.Second)
			numero_biscotti += x
			ack_deposito <- 1

		case x := <-when(numero_biscotti >= 2 && (numero_biscotti >= MAXBISCOTTI/2 || len(deposito_biscotto) == 0), prelievo_biscotti):
			fmt.Printf("[Alimentatore] richiesta di due biscotti\n")
			time.Sleep(time.Duration(rand.Intn(3)+1) * time.Second)
			numero_biscotti -= x
			ack_prelievo <- 1

		case <-termina:
			fmt.Printf("[Alimentatore]: termina\n")
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
	rand.Seed(time.Now().Unix())

	go Alimentatore()
	go Operaio()

	<-done
	<-done

	termina <- true
	<-done

	termina <- true
	<-done

	fmt.Printf("[Main]: FINE\n")
}
