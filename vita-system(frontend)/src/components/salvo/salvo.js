import React from "react";
import './salvo.css'
import {AiOutlineClose} from 'react-icons/ai'

export const Salvo = ({ isOpen, setSave }) => {
    
    const fechar = () => {setSave()}
    if(isOpen)   
        return (
            <div id="background">
                <div className="boxSalvo">
                    <h1 id='tituloSalvo'>Relatório Salvo!</h1>
                    <button id='botaosalvo' onClick={fechar}><AiOutlineClose/></button>
                </div>
            </div>
        )
        
    return null
};