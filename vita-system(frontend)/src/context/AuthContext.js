import React, { createContext, useState } from "react";
import axios from "axios";

export const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
  const [user, setUser] = useState(null);

    async function handleLogin(email,password) {
        try {
          const response = await axios.post("http://localhost:8000/login", {
            email: email,
            password: password,
    
          });
          axios.defaults.headers.common['Authorization'] = `Token ${response.data.token}`;
    
        } catch (error) {
          console.error("Erro no login:", error);
        }
      };

    async function handleLogout() {
        try {
          //await axios.post("http://localhost:8000/logout");
          axios.defaults.headers.common["Authorization"] = "";
          setUser(null);
        } catch (error) {
          console.error("Erro no logout:", error);
        }
        
      }; 
    return (

        <AuthContext.Provider value={{user,handleLogin, handleLogout}}>
            
            {children}

        </AuthContext.Provider>

    )

}