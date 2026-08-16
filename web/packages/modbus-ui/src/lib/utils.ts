import { clsx, type ClassValue } from "clsx"
import { twMerge } from "tailwind-merge"

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export function getFeatureFlag(features: { [key: string]: boolean }, flagName: string): boolean {
  return features[flagName] === true;
}
