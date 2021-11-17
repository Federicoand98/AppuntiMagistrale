package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXPROC = 100
const NB = 20
const NE = 10

// Canali
var richiesta_BT = make(chan int)
var richiesta_EB = make(chan int)
var richiesta_FLEX = make(chan int)

var rilascio_BT = make(chan int)
var rilascio_EB = make(chan int)
var rilascio_FLEX = make(chan int)

var risorsa_BT [MAXPROC]chan int
var risorsa_EB [MAXPROC]chan int

var done = make(chan int)
var termina = make(chan int)

func client(tipo int, i int) {

	if tipo == 0 {

		richiesta_BT <- i
		r := <-risorsa_BT[i]

		timeout := rand.Intn(3)
		time.Sleep(time.Duration(timeout) * time.Second)

		rilascio_BT <- r

	} else if tipo == 1 {

		richiesta_EB <- i
		r := <-risorsa_EB[i]

		timeout := rand.Intn(3)
		time.Sleep(time.Duration(timeout) * time.Second)

		rilascio_EB <- r

	} else if tipo == 2 {

		richiesta_FLEX <- i
		// to-do

	}

	done <- i
}

func server(nbt int, nbe int, nproc int) {
	var disponibili_bt int = nbt
	var disponibili_be int = nbe

	var libera_bt [NB]bool
	var libera_be [NE]bool

	var sospesi_be [MAXPROC]bool
	var sospesi_bt [MAXPROC]bool

	var nsosp_bt int = 0
	var nsosp_be int = 0

	var res, p, i int

	for i := 0; i < nbt; i++ {
		libera_bt[i] = true
	}

	for i := 0; i < nbe; i++ {
		libera_be[i] = true
	}

	for i := 0; i < nproc; i++ {
		sospesi_be[i] = false
		sospesi_bt[i] = false
	}

	for {
		select {
		case p = <-richiesta_BT:
			if disponibili_bt > 0 {
				for i = 0; i < nbt && !libera_bt[i]; i++ {

				}
				libera_bt[i] = false
				disponibili_bt--
				risorsa_BT[p] <- i
			} else {
				sospesi_bt[p] = true
				nsosp_bt++
			}
		case p = <-richiesta_EB:
			{

			}
		case p = <-richiesta_FLEX:
			{

			}
		case p = <-rilascio_BT:
			{

			}
		case p = <-rilascio_EB:
			{

			}
		case p = <-rilascio_FLEX:
			{

			}
		case p = <-termina:
			{

			}
		}
	}

}

func main() {
	fmt.Print("Ciao")
}
