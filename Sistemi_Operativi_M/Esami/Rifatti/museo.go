package main

import (
	"fmt"
	"math/rand"
	"time"
)

/////////////////////////////////////////////////////////////////////
// Costanti
/////////////////////////////////////////////////////////////////////
const MAXBUFF = 100
const MAXPROC = 10
const MAX = 5 // capacità
const M = 0
const S = 1
const MS = 2
const NM = 20
const NS = 15

var tipoUtente = [3]string{"Museo", "SalaMostre", "MuseoESalaMostre"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var entrata_visitatore [3]chan Richiesta
var uscita_visitatore [3]chan Richiesta
var entrata_operatore [2]chan Richiesta
var uscita_operatore [2]chan Richiesta

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina = make(chan bool)

/////////////////////////////////////////////////////////////////////
// Funzioni Ausiliarie
/////////////////////////////////////////////////////////////////////
// se si usa struttura dati modificare la when
func when(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func whenInt(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func sleepRandTime(timeLimit int) {
	if timeLimit > 0 {
		time.Sleep(time.Duration(rand.Intn(timeLimit)+1) * time.Second)
	}
}

func remove(slice []int, s int) []int {
	return append(slice[:s], slice[s+1:]...)
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func visitatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura

	fmt.Printf("[Visitatore %d %s]: avviato\n", id, tipoUtente[tipo])

	entrata_visitatore[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Visitatore %d %s]: entrato\n", id, tipoUtente[tipo])

	sleepRandTime(5)

	uscita_visitatore[tipo] <- richiesta
	<-richiesta.ack
	fmt.Printf("[Visitatore %d %s]: uscito\n", id, tipoUtente[tipo])

	done <- true
}

func operatore(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}
	var ris int

	fmt.Printf("[Operatore %d %s]: avviato\n", id, tipoUtente[tipo])

	for {
		entrata_operatore[tipo] <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Operatore %d %s]: termino\n", id, tipoUtente[tipo])
			done <- true
			return
		}

		fmt.Printf("[Operatore %d %s]: entrato\n", id, tipoUtente[tipo])

		sleepRandTime(5)

		uscita_operatore[tipo] <- richiesta
		ris = <-richiesta.ack

		if ris == -1 {
			fmt.Printf("[Operatore %d %s]: termino\n", id, tipoUtente[tipo])
			done <- true
			return
		}

		fmt.Printf("[Operatore %d %s]: uscito per una pausa\n", id, tipoUtente[tipo])

		sleepRandTime(5)
	}
}

func gestore() {
	var running bool = true
	var num_o_m int = 0
	var num_o_s int = 0
	var num_v_m int = 0
	var num_v_s int = 0

	for {
		select {
		case richiesta := <-when(running && num_v_m < NM && num_o_m > 0, entrata_visitatore[M]):
			num_v_m++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: entrato visitatore nel museo. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running && num_v_s < NS && num_o_s > 0 && len(entrata_visitatore[M]) == 0, entrata_visitatore[S]):
			num_v_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: entrato visitatore nella sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running && num_v_m < NM && num_v_s < NS && num_o_s > 0 && num_o_m > 0 && len(entrata_visitatore[M]) == 0 && len(entrata_visitatore[S]) == 0, entrata_visitatore[MS]):
			num_v_m++
			num_v_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: entrato visitatore nel museo+sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running, uscita_visitatore[M]):
			num_v_m--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscito visitatore dal museo. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running, uscita_visitatore[S]):
			num_v_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscito visitatore dalla sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running, uscita_visitatore[MS]):
			num_v_m--
			num_v_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscito visitatore dal museo+sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running, entrata_operatore[M]):
			num_o_m++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: entrato operatore nel museo. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running, entrata_operatore[S]):
			num_o_s++
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: entrato operatore nella sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running && (num_o_m > 1 || num_o_m == 1 && num_v_m == 0), uscita_operatore[M]):
			num_o_m--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscito operatore dal museo. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(running && (num_o_s > 1 || num_o_s == 1 && num_v_s == 0), uscita_operatore[S]):
			num_o_s--
			richiesta.ack <- 1
			fmt.Printf("[Gestore]: uscito operatore dalla sala. NumVm: %d, NumVs: %d, NumOm: %d, NumOs: %d\n", num_v_m, num_v_s, num_o_m, num_o_s)
		case richiesta := <-when(!running, entrata_operatore[M]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, entrata_operatore[S]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, uscita_operatore[M]):
			richiesta.ack <- -1
		case richiesta := <-when(!running, uscita_operatore[S]):
			richiesta.ack <- -1
		case <-termina:
			if running == true {
				fmt.Printf("[Gestore]: inizio terminazione\n")
				running = false
			} else {
				fmt.Printf("[Gestore]: fine\n")
				done <- true
				return
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	var vis int = 50
	var op int = 8

	//inizializzazione canali se non si usa struttura dati
	for i := 0; i < 3; i++ {
		entrata_visitatore[i] = make(chan Richiesta, MAXBUFF)
		uscita_visitatore[i] = make(chan Richiesta, MAXBUFF)
	}

	for i := 0; i < 2; i++ {
		entrata_operatore[i] = make(chan Richiesta, MAXBUFF)
		uscita_operatore[i] = make(chan Richiesta, MAXBUFF)
	}

	go gestore()

	for i := 0; i < op/2; i++ {
		go operatore(i, M)
	}

	for i := 0; i < op/2; i++ {
		go operatore(i, S)
	}

	for i := 0; i < vis; i++ {
		go visitatore(i, rand.Intn(3))
	}

	// join
	for i := 0; i < vis; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < op; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
