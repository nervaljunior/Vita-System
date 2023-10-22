import { useState } from "react";

export const useLocalStorage = (keyName, defaultValue) => {
  const [storedValue, setStoredValue] = useState(() => {
    try {
      const value = window.localStorage.getItem(keyName);
      if (value) {
        const parsedValue = JSON.parse(value);
        return parsedValue.role !== undefined ? parsedValue.role : defaultValue;
      } else {
        window.localStorage.setItem(keyName, JSON.stringify({ role: defaultValue }));
        return defaultValue;
      }
    } catch (err) {
      return defaultValue;
    }
  });

  const setValue = (newValue) => {
    try {
      const valueToStore = { role: newValue };
      window.localStorage.setItem(keyName, JSON.stringify(valueToStore));
      setStoredValue(valueToStore.role);
    } catch (err) {}
  };

  return [storedValue, setValue];
};
