import { Navigate } from "react-router-dom";
import { useAuth } from "./context"
import React from "react";

export const ProtectedRoute = ({ children }) => {
    const { role } = useAuth();
    if(!role) {
        return <Navigate to="/login" />
    } else if (role === 'Administrator'){
        return children
    } else if (role !== 'Administrator') {
        return <Navigate to="/func" />
    }
}; 

export const ProtectedRouteFunc = ({ children }) => {
    const { role } = useAuth();
    if(!role) {
        return <Navigate to="/login" />
    } else if (role === 'User'){
        return children
    } else if (role !== 'User') {
        return <Navigate to="/adm" />
    }
}; 