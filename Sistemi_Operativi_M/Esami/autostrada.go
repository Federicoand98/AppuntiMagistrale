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
const MAXPROC = 100
const MAX = 10
const NS = 0
const SN = 1
const SUD = 0
const NORD = 1

var corsia = [2]string{"Sud", "Nord"}

/////////////////////////////////////////////////////////////////////
// Strutture Dati
/////////////////////////////////////////////////////////////////////
type Richiesta struct {
	id  int
	ack chan int
}

type Info struct {
	id        int
	corsia    int
	direzione int
}

/////////////////////////////////////////////////////////////////////
// Canali
/////////////////////////////////////////////////////////////////////
var entrata_NS = make(chan Richiesta, MAXBUFF)
var entrata_SN = make(chan Richiesta, MAXBUFF)
var uscita_NS = make(chan Richiesta, MAXBUFF)
var uscita_SN = make(chan Richiesta, MAXBUFF)

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

func remove(slice []Info, s int) []Info {
	return append(slice[:s], slice[s+1:]...)
}

/////////////////////////////////////////////////////////////////////
// Goroutine
/////////////////////////////////////////////////////////////////////
func goroutine(id int, tipo int) {
	richiesta := Richiesta{id, make(chan int)} // se si usa struttura
	var ris int

	if tipo == NS {
		fmt.Printf("[Auto %d]: avviata auto N->S\n", id)

		entrata_NS <- richiesta
		ris = <-richiesta.ack
		fmt.Printf("[Auto %d]: auto entrata in corsia %s in direzione N->S\n", id, corsia[ris])

		sleepRandTime(7)

		uscita_NS <- richiesta
		ris = <-richiesta.ack
		fmt.Printf("[Aurto %d]: uscita dalla corsia %s, ha percorso la direzione N->S\n", id, corsia[ris])
	} else if tipo == SN {
		fmt.Printf("[Auto %d]: avviata auto N->S\n", id)

		entrata_SN <- richiesta
		ris = <-richiesta.ack
		fmt.Printf("[Auto %d]: auto entrata in corsia %s in direzione S->N\n", id, corsia[ris])

		sleepRandTime(7)

		uscita_SN <- richiesta
		ris = <-richiesta.ack
		fmt.Printf("[Aurto %d]: uscita dalla corsia %s, ha percorso la direzione S->N\n", id, corsia[ris])
	}

	done <- true
}

func autostrada() {
	var num_sud int = 0
	var num_nord int = 0
	var num_nord_alt int = 0
	var info = make([]Info, MAXPROC)
	var counter int = 0
	var trovato bool = false

	for {
		select {
		case richiesta := <-when(((num_sud < MAX) || (num_sud == MAX && num_nord == 0 && num_nord_alt < MAX && len(entrata_SN) == 0)), entrata_NS):
			if num_sud < MAX {
				num_sud++
				inf := Info{richiesta.id, SUD, NS}
				info[counter] = inf
				counter++
				richiesta.ack <- SUD
				fmt.Printf("[AUTOSTRADA]: auto %d entrata in corsia SUD in dir N->S. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
			} else {
				num_nord_alt++
				inf := Info{richiesta.id, NORD, NS}
				info[counter] = inf
				counter++
				richiesta.ack <- NORD
				fmt.Printf("[AUTOSTRADA]: auto %d entrata in corsia NORD alternata in dir N->S. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
			}
		case richiesta := <-when((num_nord < MAX && num_nord_alt == 0), entrata_SN):
			num_nord++
			i := Info{richiesta.id, NORD, SN}
			info[counter] = i
			counter++
			richiesta.ack <- NORD
			fmt.Printf("[AUTOSTRADA]: auto %d entrata in corsia NORD normale in dir S->N. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
		case richiesta := <-when(true, uscita_NS):
			var i int = 0
			for i = 0; i < len(info) && !trovato; i++ {
				if info[i].id == richiesta.id {
					trovato = true
				}
			}
			if info[i].corsia == SUD {
				num_sud--
				richiesta.ack <- SUD
				fmt.Printf("[AUTOSTRADA]: auto %d uscita dalla corsia SUD normale in dir N->S. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
			} else if info[i].corsia == NORD && info[i].direzione == NS {
				num_nord_alt--
				richiesta.ack <- NORD
				fmt.Printf("[AUTOSTRADA]: auto %d uscita dalla corsia NORD normale in dir N->S. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
			}
			info = remove(info, i)
			trovato = false
		case richiesta := <-when(true, uscita_SN):
			var i int = 0
			for i = 0; i < len(info) && !trovato; i++ {
				if info[i].id == richiesta.id {
					trovato = true
				}
			}
			info = remove(info, i)
			num_nord--
			trovato = false
			richiesta.ack <- NORD
			fmt.Printf("[AUTOSTRADA]: auto %d uscita dalla corsia NORD normale in dir S->N. NumSud: %d, NumNord: %d, NumNordAlt: %d\n", richiesta.id, num_sud, num_nord, num_nord_alt)
		case <-termina:
			fmt.Println("\n\n[SERVER]: Fine!")
			done <- true
			return
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////
func main() {
	rand.Seed(time.Now().Unix())

	var N int = 20
	var S int = 45

	go autostrada()

	for i := 0; i < N; i++ {
		go goroutine(i, SN)
	}

	for i := N; i < S; i++ {
		go goroutine(i, NS)
	}

	// join
	for i := 0; i < N+S; i++ {
		<-done
	}

	termina <- true
	<-done
	fmt.Printf("\n[Main]: fine.\n")
}
