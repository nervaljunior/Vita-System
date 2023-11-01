import { createContext, useState } from "react";
import axios from "axios";

export const AuthContext = createContextContext;

export function AuthContextProvider({children}){

    const[user,setUser]=useState();

    async function handleLogin(email,password) {
        try {
          const response = await axios.post("http://localhost:7777/login", {
            email,
            password,
    
          });
          axios.defaults.headers.common['Authorization'] = `Token ${resp.data.token}`;
    
        } catch (error) {
          console.error("Erro no login:", error);
        }
      };

    return (

        <AuthContext.Provider value={{user}}>
            
            {children}

        </AuthContext.Provider>

    )

}