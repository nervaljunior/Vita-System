import React from "react";
import './configAntena.css'
import { AiOutlineClose } from 'react-icons/ai'
import { useState } from "react";
import axios from "axios";
import { Funcionario } from "../../pages/func/func";


export const ConfigAntena = ({isAntena, isOpen, setConfigAntena, setGerarGrafico1, setGerarGrafico2, setPararCronometro, setIniciarCronometro}) => {
    
    const [port, setPort] = useState("")
    const [bautrate, setBautrate] = useState("")
    const [bytesize, setBytesize] = useState("")
    const [parit, setParit] = useState("")
    const [stopbits, setStopbits] = useState("")
    const [erroAntena, setErroAntena] = useState(false)

    // Postar dados da Antena
    const postConfigAntena = () => {
        if(isAntena === "antena1") {
            axios.post("http://localhost:4000/configantena/antena1", {
                port,
                bautrate,
                bytesize,
                parit,
                stopbits,
            })
            axios.post("http://localhost:4000/configantena/stop1", {
                stop: false
            });

            setTimeout(() => axios.get("http://localhost:4000/configantena")
                .then(response => {
                    const { mensagem1: apiData } = response.data;
                    setErroAntena(apiData);
                    if(apiData == false) {
                        setGerarGrafico1(true);
                        setConfigAntena(false);
                        setIniciarCronometro(true);
                        setPararCronometro(false); 
                    }
                }), 3000);
        }

        else if (isAntena === "antena2") {
            axios.post("http://localhost:4000/configantena/antena2", {
                port,
                bautrate,
                bytesize,
                parit,
                stopbits,
            })
            axios.post("http://localhost:4000/configantena/stop2", {
                stop: false
            });

            setTimeout(() => axios.get("http://localhost:4000/configantena")
                .then(response => {
                    const { mensagem2: apiData } = response.data;
                    setErroAntena(apiData);
                    if(apiData == false) {
                        setGerarGrafico2(true);
                        setConfigAntena(false);
                        setIniciarCronometro(true);
                        setPararCronometro(false); 
                    }
                }), 3000);
        }

        limparCampos();
    }

    
    // limpar campos
    const limparCampos = () => {
        setPort("");
        setBautrate("");
        setBytesize("");
        setParit("");
        setStopbits("");
    }

    if(isOpen)   
        return (
            <div id="background">
                <div id="pagecad" className="boxantena">
                    <h1 id='h1configantena'>Configurações de Comunicação</h1>
                    <input
                    type="text"
                    className="form"
                    id="port"
                    placeholder="PORT"
                    value={port}
                    onChange={(e) => setPort(e.target.value)}
                    />
                    <input
                    type="number"
                    className="form"
                    id="bautrate"
                    placeholder="BAUDRATE"
                    value={bautrate}
                    onChange={(e) => setBautrate(e.target.value)}
                    />
                    <input
                    type="number"
                    className="form"
                    id="bytesize"
                    placeholder="BYTESIZE"
                    value={bytesize}
                    onChange={(e) => setBytesize(e.target.value)}
                    />
                    <input
                    type="text"
                    className="form"
                    id="parit"
                    placeholder="PARIT"
                    value={parit}
                    onChange={(e) => setParit(e.target.value)}
                    />
                    <input
                    type="number"
                    className="form"
                    id="stopbits"
                    placeholder="STOPBITS"
                    value={stopbits}
                    onChange={(e) => setStopbits(e.target.value)}
                    />
                    
                    <button type="submit" id="buttoncomecar" onClick={postConfigAntena}>
                    COMEÇAR 
                    </button>

                    <button type="submit" id="buttonfechar" onClick={setConfigAntena}>
                    <span><AiOutlineClose/></span>
                    </button>

                    {erroAntena && <p id='erroantena'>Algo deu errado (Tente Novamente)</p>}
                </div>
            </div>
        )
        
    return null
};