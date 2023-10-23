import { createContext, useContext, useMemo } from "react";
import { useNavigate } from "react-router-dom";
import { useLocalStorage } from "./useLocalStorage"
import React from 'react';
import axios from 'axios';

const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
  const [role, setRole] = useLocalStorage("role", null);
  const navigate = useNavigate();

  // Função de login no contexto
  const login = async (data) => {
    try {
      // Fazer a solicitação de login para o servidor backend
      const response = await axios.post("http://localhost:5000/login", data);
      // Verificar a resposta do servidor
      if (response.data.message === "Login successful") {
        setRole(response.data.role);
        return response; // Retornar a resposta do servidor, se necessário
      } else {
        throw new Error("Login failed");
      }
    } catch (error) {
      throw new Error("Login failed");
    }
  };
    
   //chama essa funcao para fazer o logout
    const logout = () => {
        setRole(null);
        navigate("/", { replace: true });
  };

    const value = useMemo(
        () => (
            {role, login, logout}),
            [role]
        
    );
    
    return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>

};

export const useAuth = () => {
    return useContext(AuthContext);
};