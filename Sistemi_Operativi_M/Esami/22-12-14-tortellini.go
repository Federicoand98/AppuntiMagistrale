package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXCLI = 100
const MAXSFO = 50
const MAXPREN = 60
const MAXFRIGO = 20
const MAXBUFF = 100

type Richiesta struct {
	id     int
	ticket int
}

//definizione canali
var done = make(chan bool)
var termina = make(chan bool)
var prenota = make(chan int, MAXBUFF)
var ritira = make(chan Richiesta, MAXBUFF)
var deposita = make(chan int, MAXBUFF)
var ack_prenota [MAXCLI]chan int
var ack_ritira [MAXCLI]chan bool
var ack_deposita [MAXSFO]chan bool
var r int

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func whenR(b bool, c chan Richiesta) chan Richiesta {
	if !b {
		return nil
	}
	return c
}

func cliente(id int) {
	var risP int
	var answ bool
	var ritiro Richiesta

	fmt.Printf("Cliente %d inizio.", id)

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	prenota <- id
	risP = <-ack_prenota[id]
	fmt.Printf("[cliente %d]  richiesta prenotazione - ottenuto ticket %d\n ", id, risP)

	if risP == -1 {
		done <- true
		return
	}

	time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

	ritiro = Richiesta{id, risP}
	ritira <- ritiro
	answ = <-ack_ritira[id]

	if answ {
		fmt.Printf("[cliente %d]  ritirati tortellini!\n ", id)
	} else {
		fmt.Printf("[cliente %d]  ritiro negato..\n ", id)
	}

	done <- true
	return
}

func sfoglina(id int) {
	var esito bool

	for {
		time.Sleep(time.Duration(rand.Intn(5)+1) * time.Second)

		deposita <- id
		esito = <-ack_deposita[id]

		if esito {
			fmt.Printf("[sfoglina %d]  ho depositato una nuova confezione.\n ", id)
		} else {
			fmt.Printf("[sfoglina %d]  termino\n ", id)
			done <- true
			return
		}
	}
}

func server(cli int, sfo int) {
	var contatore int // valore ticket corrente
	var daritirare int
	var fine bool
	var ticket [MAXCLI]int //ticket assegnati
	var ris int            //messaggio di risposta prenotazione
	var rr bool            //messaggio di risposta ritiro
	var infrigo int
	var i int
	var pre int
	contatore = 1
	fine = false
	infrigo = 0
	pre = -1
	fmt.Printf("\n*** TEST con %d CLIENTI e %d SFOGLINE ***\n ", cli, sfo)

	for {
		select {
		case x := <-when(((infrigo < MAXFRIGO) && (fine == false)), deposita):
			infrigo++
			fmt.Printf("[server]  depositata nuova confezione da sfoglina %d !", x)
			fmt.Printf("(infrigo=%d, daritirare=%d)!  \n", infrigo, daritirare)
			ack_deposita[x] <- true

		case x := <-when((fine == true), deposita):
			fmt.Printf("[server]  termino la sfoglina %d !", x)
			fmt.Printf("(infrigo=%d, daritirare=%d)!  \n", infrigo, daritirare)
			ack_deposita[x] <- false

		case x := <-prenota:
			if contatore < MAXPREN {
				ticket[x] = contatore
				ris = contatore
				contatore++
				daritirare++
				fmt.Printf("[server] prenotata confezione per cliente %d: assegnato ticket %d ", x, ris)
				fmt.Printf("(infrigo=%d, daritirare=%d)!  \n", infrigo, daritirare)
			} else {
				ris = -1
				fmt.Printf("[server] rifiutata prenotazione per cliente %d: assegnato ticket %d  \n", x, ris)
			}
			ack_prenota[x] <- ris

		case x := <-whenR((infrigo > 0) && (len(prenota) == 0), ritira):
			for i = 0; i < cli; i++ {
				if x.ticket == ticket[i] {
					pre = i
				}
			}

			if pre >= 0 {
				infrigo--
				daritirare--
				ticket[pre] = 0
				rr = true
				fmt.Printf("[server]  cliente %d ha ritirato", i)
				fmt.Printf("(infrigo=%d, daritirare=%d)!  \n", infrigo, daritirare)
				if daritirare == 0 {
					fine = true
					fmt.Printf("[server]  completati ritiri")
					fmt.Printf("(infrigo=%d, daritirare=%d)!\n", infrigo, daritirare)
				}
			} else {
				rr = false
				fmt.Printf("[server]  cliente %d - RITIRO NEGATO!\n", i)
			}
			ack_ritira[x.id] <- rr

		case <-termina:
			fmt.Printf("[server] FINE (sono rimaste %d confezioni in frigo..)\n", infrigo)
			done <- true
			return
		}
	}
}

func main() {

}
