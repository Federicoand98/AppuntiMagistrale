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
const A = 0
const B = 1
const T1 = 0
const T2 = 1

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
// con più accorgimenti ed astrazioni si potrebbe fare con MOLTI meno canali, per semplificare la logica e il server faccio un canale per tipo di richiesta
var entra_tratto_t1_SN = make(chan Richiesta, MAXBUFF)
var entra_tratto_t2_SN = make(chan Richiesta, MAXBUFF)
var entra_tratto_t2_NS = make(chan Richiesta, MAXBUFF)
var entra_tratto_t1_NS = make(chan Richiesta, MAXBUFF)
var entra_guida_t1_SN = make(chan Richiesta, MAXBUFF)
var entra_guida_t2_SN = make(chan Richiesta, MAXBUFF)
var entra_guida_t2_NS = make(chan Richiesta, MAXBUFF)
var entra_guida_t1_NS = make(chan Richiesta, MAXBUFF)
var piazzola_turista_SN = make(chan Richiesta, MAXBUFF)
var piazzola_turista_NS = make(chan Richiesta, MAXBUFF)
var piazzola_guida_SN = make(chan Richiesta, MAXBUFF)
var piazzola_guida_NS = make(chan Richiesta, MAXBUFF)
var arrivo_turista_SN = make(chan Richiesta, MAXBUFF)
var arrivo_turista_NS = make(chan Richiesta, MAXBUFF)
var arrivo_guida_SN = make(chan Richiesta, MAXBUFF)
var arrivo_guida_NS = make(chan Richiesta, MAXBUFF)

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
func turista(id int, tratto int) {
	richiesta := Richiesta{id, make(chan int)}

	if tratto == A {
		fmt.Printf("[Turista %d]: avviato, percorrerà il tratto S->N\n", id)

		entra_tratto_t1_SN <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sta percorrendo il tratto T1 in direzione S->N\n", id)

		sleepRandTime(3)

		piazzola_turista_SN <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sono entrato in piazzola, ora sosto\n", id)

		sleepRandTime(10)

		entra_tratto_t2_SN <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sta percorrendo il tratto T2 in direzione S->N\n", id)

		sleepRandTime(3)

		arrivo_turista_SN <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: uscito dal tratto T2 in direzione S->N\n", id)

	} else if tratto == B {
		fmt.Printf("[Turista %d]: avviato, percorrerà il tratto N->S\n", id)

		entra_tratto_t2_NS <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sta percorrendo il tratto T2 in direzione N->S\n", id)

		sleepRandTime(3)

		piazzola_turista_NS <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sono entrato in piazzola, ora sosto\n", id)

		sleepRandTime(10)

		entra_tratto_t1_NS <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: sta percorrendo il tratto T1 in direzione N->S\n", id)

		sleepRandTime(3)

		arrivo_turista_NS <- richiesta
		<-richiesta.ack
		fmt.Printf("[Turista %d]: uscito dal tratto T1 in direzione N->S\n", id)
	}

	done <- true
}

func guida(id int, dir int) {
	richiesta := Richiesta{id, make(chan int)}

	for {
		if dir == A {
			fmt.Printf("[Guida %d]: avviato, percorrerà il tratto S->N\n", id)

			entra_guida_t1_SN <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sta percorrendo il tratto T1 in direzione S->N\n", id)

			sleepRandTime(3)

			piazzola_guida_SN <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sono entrato in piazzola, ora sosto\n", id)

			sleepRandTime(10)

			entra_guida_t2_SN <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sta percorrendo il tratto T2 in direzione S->N\n", id)

			sleepRandTime(3)

			arrivo_guida_SN <- richiesta
			ris := <-richiesta.ack
			fmt.Printf("[Guida %d]: uscito dal tratto T2 in direzione S->N\n", id)

			if ris == -1 {
				fmt.Printf("[Guida %d]: termino\n", id)
				done <- true
				return
			}

			dir = B

		} else if dir == B {
			fmt.Printf("[Guida %d]: avviato, percorrerà il tratto N->S\n", id)

			entra_guida_t2_NS <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sta percorrendo il tratto T2 in direzione N->S\n", id)

			sleepRandTime(3)

			piazzola_guida_NS <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sono entrato in piazzola, ora sosto\n", id)

			sleepRandTime(10)

			entra_guida_t1_NS <- richiesta
			<-richiesta.ack
			fmt.Printf("[Guida %d]: sta percorrendo il tratto T1 in direzione N->S\n", id)

			sleepRandTime(3)

			arrivo_guida_NS <- richiesta
			ris := <-richiesta.ack
			fmt.Printf("[Guida %d]: uscito dal tratto T1 in direzione N->S\n", id)

			if ris == -1 {
				fmt.Printf("[Guida %d]: termino\n", id)
				done <- true
				return
			}

			dir = A
		}
	}
}

func server() {
	var running bool = true
	var guide_t1 int = 0
	var guide_t2 int = 0
	var turisti_t1 int = 0
	var turisti_t2 int = 0
	var num_piazzola int = 0
	var t1_occupato_SN bool = false
	var t2_occupato_SN bool = false
	var t1_occupato_NS bool = false
	var t2_occupato_NS bool = false

	for {
		select {
		case richiesta := <-when(running && !t1_occupato_NS && guide_t1 > turisti_t1 && len(entra_guida_t1_SN) == 0 && len(entra_tratto_t1_NS) == 0 && len(entra_guida_t1_NS) == 0, entra_tratto_t1_SN):
			t1_occupato_SN = true
			turisti_t1++
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore S->N entrato in T1. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t2_occupato_NS && guide_t2 > turisti_t2 && len(entra_guida_t2_SN) == 0, entra_tratto_t2_SN):
			t2_occupato_SN = true
			num_piazzola--
			turisti_t2++
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore S->N entrato in T2. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t2_occupato_SN && guide_t2 > turisti_t2 && len(entra_guida_t2_NS) == 0 && len(entra_guida_t2_SN) == 0 && len(entra_tratto_t2_SN) == 0, entra_tratto_t2_NS):
			t2_occupato_NS = true
			turisti_t2++
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore N->S entrato in T2. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t1_occupato_SN && guide_t1 > turisti_t1 && len(entra_guida_t1_NS) == 0, entra_tratto_t1_NS):
			t1_occupato_NS = true
			num_piazzola--
			turisti_t1++
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore N->S entrato in T1. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && num_piazzola < MAX, piazzola_turista_SN):
			num_piazzola++
			turisti_t1--
			if turisti_t1 == 0 && guide_t1 == 0 {
				t1_occupato_SN = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore S->N entrato in piazzola. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && num_piazzola < MAX, piazzola_turista_NS):
			num_piazzola++
			turisti_t2--
			if turisti_t2 == 0 && guide_t2 == 0 {
				t2_occupato_NS = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: visitatore N->S entrato in piazzola. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t1_occupato_NS && len(entra_guida_t1_NS) == 0 && len(entra_tratto_t1_NS) == 0, entra_guida_t1_SN):
			t1_occupato_SN = true
			guide_t1++
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida S->N entrato in T1. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t2_occupato_NS, entra_guida_t2_SN):
			t2_occupato_SN = true
			guide_t2++
			num_piazzola--
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida S->N entrato in T2. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t2_occupato_SN && len(entra_guida_t2_SN) == 0 && len(entra_tratto_t2_SN) == 0, entra_guida_t2_NS):
			t2_occupato_NS = true
			guide_t2++
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida N->S entrato in T2. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && !t1_occupato_SN, entra_guida_t1_NS):
			t1_occupato_NS = true
			guide_t1++
			num_piazzola--
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida N->S entrato in T1. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && num_piazzola < MAX, piazzola_guida_SN):
			num_piazzola++
			guide_t1--
			if turisti_t1 == 0 && guide_t1 == 0 {
				t1_occupato_SN = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida S->N entrato in piazzola. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running && num_piazzola < MAX, piazzola_guida_NS):
			num_piazzola++
			guide_t2--
			if turisti_t2 == 0 && guide_t2 == 0 {
				t2_occupato_NS = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida N->S entrato in piazzola. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running, arrivo_turista_SN):
			turisti_t2--
			if turisti_t2 == 0 && guide_t2 == 0 {
				t2_occupato_SN = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: turista S->N finito. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running, arrivo_turista_NS):
			turisti_t1--
			if turisti_t1 == 0 && guide_t1 == 0 {
				t1_occupato_NS = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: turista N->S finito. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running, arrivo_guida_SN):
			guide_t2--
			if turisti_t2 == 0 && guide_t2 == 0 {
				t2_occupato_SN = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida S->N finito. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(running, arrivo_guida_NS):
			guide_t1--
			if turisti_t1 == 0 && guide_t1 == 0 {
				t1_occupato_NS = false
			}
			richiesta.ack <- 1
			fmt.Printf("[Server]: guida N->S finito. NumT1: %d, NumT2: %d, GuideT1: %d, GuideT2: %d, NumPiazzola: %d.", turisti_t1, turisti_t2, guide_t1, guide_t2, num_piazzola)
		case richiesta := <-when(!running, arrivo_guida_SN):
			richiesta.ack <- -1
		case richiesta := <-when(!running, arrivo_guida_NS):
			richiesta.ack <- -1
		case <-termina:
			if running {
				fmt.Println("[Addetto]: start termination")
				running = false
			} else {
				fmt.Println("[Addetto]: terminato")
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

	var n_t_sn int = 20
	var n_g_sn int = 15
	var n_t_ns int = 22
	var n_g_ns = 14

	go server()

	for i := 0; i < n_t_sn; i++ {
		go turista(i, 0)
	}

	for i := 0; i < n_t_ns; i++ {
		go turista(i, 1)
	}

	for i := 0; i < n_g_sn; i++ {
		go guida(i, 0)
	}

	for i := 0; i < n_g_ns; i++ {
		go guida(i, 1)
	}

	for i := 0; i < n_t_ns+n_t_sn; i++ {
		<-done
	}

	termina <- true

	for i := 0; i < n_g_sn+n_g_ns; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
