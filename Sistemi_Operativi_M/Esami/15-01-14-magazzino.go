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
const MAX = 30
const MARTELLO = 0
const TENAGLIA = 1
const BADILE = 2
const K = 5

var tipoUtensili = [3]string{"MARTELLI", "TENAGLIE", "BADILI"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

type Rifornimento struct {
	id  int
	num int
	ack chan int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var acquisto_negozianti [3]chan Richiesta
var acquisto_privati [3]chan Richiesta
var rifornimento_fornitori [3]chan Rifornimento

/////////////////////////////////////////////////////////////////////
// Join Goroutine
/////////////////////////////////////////////////////////////////////
var done = make(chan bool)
var termina_magazzino = make(chan bool)

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

func whenRifornimento(b bool, c chan Rifornimento) chan Rifornimento {
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

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func negoziante(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	acquisto_negozianti[tipo] <- richiesta
	ris := <-richiesta.ack

	if ris == 1 {
		fmt.Printf("[Cliente Negoziante %d]: acquistati %d %s\n", id, K, tipoUtensili[tipo])
	} else {
		fmt.Printf("[Cliente Negoziante %d]: acquisto non accettato\n", id)
	}

	done <- true
	return
}

func privato(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)}

	acquisto_privati[tipo] <- richiesta
	ris := <-richiesta.ack

	if ris == 1 {
		fmt.Printf("[Cliente Privato %d]: acquistati un %s\n", id, tipoUtensili[tipo])
	} else {
		fmt.Printf("[Cliente Privato %d]: acquisto non accettato\n", id)
	}

	done <- true
	return
}

func fornitore(id int, tipo int) {
	richiesta := Rifornimento{id, rand.Intn(MAX), make(chan int)}

	for {
		rifornimento_fornitori[tipo] <- richiesta
		ris := <-richiesta.ack

		if ris == 1 {
			fmt.Printf("[Fornitore %d]: rifornimento di %d %s\n", id, richiesta.num, tipoUtensili[tipo])
		} else if ris == -1 {
			fmt.Printf("[Fornitore %d]: termino..\n", id)
			done <- true
			return
		} else {
			fmt.Printf("[Fornitore %d]: rifornimento non accettato\n", id)
		}

		sleepRandTime(10)
	}
}

func magazzino() {
	var num_m int = 0
	var num_t int = 0
	var num_b int = 0
	var running bool = true

	for {
		select {
		case richiesta := <-whenRifornimento(running, rifornimento_fornitori[MARTELLO]):
			fmt.Printf("[Magazzino]: richiesta di rifornimento di %d martelli. Num martelli in magazzino: %d\n", richiesta.num, num_m)
			if num_m+richiesta.num <= MAX {
				num_m += richiesta.num
				fmt.Printf("[Magazzino]: rifornito di %d martelli. Num Martelli: %d\n", richiesta.num, num_m)
				richiesta.ack <- 1
			} else {
				richiesta.ack <- 0
			}

		case richiesta := <-whenRifornimento(running && len(rifornimento_fornitori[MARTELLO]) == 0, rifornimento_fornitori[TENAGLIA]):
			fmt.Printf("[Magazzino]: richiesta di rifornimento di %d tenaglie. Num tenaglie in magazzino: %d\n", richiesta.num, num_t)
			if num_t+richiesta.num <= MAX {
				num_t += richiesta.num
				fmt.Printf("[Magazzino]: rifornito di %d tenaglie. Num tenaglie: %d\n", richiesta.num, num_t)
				richiesta.ack <- 1
			} else {
				richiesta.ack <- 0
			}

		case richiesta := <-whenRifornimento(running && len(rifornimento_fornitori[MARTELLO]) == 0 && len(rifornimento_fornitori[TENAGLIA]) == 0, rifornimento_fornitori[BADILE]):
			fmt.Printf("[Magazzino]: richiesta di rifornimento di %d badili. Num martelli in magazzino: %d\n", richiesta.num, num_b)
			if num_b+richiesta.num <= MAX {
				num_b += richiesta.num
				fmt.Printf("[Magazzino]: rifornito di %d badili. Num badili: %d\n", richiesta.num, num_b)
				richiesta.ack <- 1
			} else {
				richiesta.ack <- 0
			}

		case richiesta := <-when(running && num_m >= K, acquisto_negozianti[MARTELLO]):
			num_m -= K
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: negoziante %d ha acquistato %d martelli. Num martelli in magazzino: %d\n", richiesta.id, K, num_m)

		case richiesta := <-when(running && num_t >= K, acquisto_negozianti[TENAGLIA]):
			num_t -= K
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: negoziante %d ha acquistato %d tenaglie. Num tenaglie in magazzino: %d\n", richiesta.id, K, num_t)

		case richiesta := <-when(running && num_b >= K, acquisto_negozianti[BADILE]):
			num_m -= K
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: negoziante %d ha acquistato %d badili. Num badili in magazzino: %d\n", richiesta.id, K, num_b)

		case richiesta := <-when(running && num_m > 0 && len(acquisto_negozianti[MARTELLO]) == 0 && len(acquisto_negozianti[TENAGLIA]) == 0 && len(acquisto_negozianti[BADILE]) == 0, acquisto_privati[MARTELLO]):
			num_m--
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: privato %d ha acquistato un martello. Num martelli in magazzino: %d\n", richiesta.id, num_m)

		case richiesta := <-when(running && num_t > 0 && len(acquisto_negozianti[MARTELLO]) == 0 && len(acquisto_negozianti[TENAGLIA]) == 0 && len(acquisto_negozianti[BADILE]) == 0, acquisto_privati[TENAGLIA]):
			num_t--
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: privato %d ha acquistato una tenaglia. Num tenaglie in magazzino: %d\n", richiesta.id, num_t)

		case richiesta := <-when(running && num_b > 0 && len(acquisto_negozianti[MARTELLO]) == 0 && len(acquisto_negozianti[TENAGLIA]) == 0 && len(acquisto_negozianti[BADILE]) == 0, acquisto_privati[BADILE]):
			num_b--
			richiesta.ack <- 1
			fmt.Printf("[Magazzino]: privato %d ha acquistato un badile. Num badili in magazzino: %d\n", richiesta.id, num_b)

		case richiesta := <-whenRifornimento(!running, rifornimento_fornitori[MARTELLO]):
			richiesta.ack <- -1
		case richiesta := <-whenRifornimento(!running, rifornimento_fornitori[TENAGLIA]):
			richiesta.ack <- -1
		case richiesta := <-whenRifornimento(!running, rifornimento_fornitori[BADILE]):
			richiesta.ack <- -1

		case <-termina_magazzino:
			if running {
				fmt.Printf("[Magazzino]: inizio terminazione\n")
				running = false
			} else {
				fmt.Println("\n[Magazzino]: Fine!")
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

	var fornitori int = 8
	var privati int = 20
	var negozianti int = 30

	for i := 0; i < 3; i++ {
		acquisto_negozianti[i] = make(chan Richiesta, MAXBUFF)
		acquisto_privati[i] = make(chan Richiesta, MAXBUFF)
		rifornimento_fornitori[i] = make(chan Rifornimento, MAXBUFF)
	}

	go magazzino()

	for i := 0; i < fornitori; i++ {
		go fornitore(i, rand.Intn(3))
	}

	for i := 0; i < privati; i++ {
		go privato(i, rand.Intn(3))
	}

	for i := 0; i < negozianti; i++ {
		go negoziante(i, rand.Intn(3))
	}

	for i := 0; i < privati+negozianti; i++ {
		<-done
	}

	termina_magazzino <- true

	for i := 0; i < fornitori; i++ {
		<-done
	}

	termina_magazzino <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
