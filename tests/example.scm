;;;; This file is part of Dico.
;;;; Copyright (C) 2008 Sergey Poznyakoff
;;;; 
;;;; Dico is free software; you can redistribute it and/or modify
;;;; it under the terms of the GNU General Public License as published by
;;;; the Free Software Foundation; either version 3, or (at your option)
;;;; any later version.
;;;; 
;;;; Dico is distributed in the hope that it will be useful,
;;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;;; GNU General Public License for more details.
;;;; 
;;;; You should have received a copy of the GNU General Public License
;;;; along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

(define-module (example)
  #:use-module (guile-user)
  #:use-module (ice-9 syncase)
  #:use-module (ice-9 format))

(define-syntax db:get
  (syntax-rules (info descr name corpus)
    ((db:get dbh name)
     (list-ref dbh 0))
    ((db:get dbh descr)
     (list-ref dbh 1))
    ((db:get dbh info)
     (list-ref dbh 2))
    ((db:get dbh corpus)
     (list-tail dbh 3))))

(define (mapcan fun list)
  (apply (lambda ( . slist)
           (let loop ((elt '())
                      (slist slist))
             (cond
              ((null? slist)
               (reverse elt))
              ((not (car slist))
               (loop elt (cdr slist)))
              (else
               (loop (cons (car slist) elt) (cdr slist))))))
         (map fun list)))

(define (open-module name . rest)
  (let ((args (cdr rest)))
    (cond
     ((null? args)
      (format (current-error-port) "open-module: not enough arguments\n")
      #f)
     ((not (null? (cdr args)))
      (format (current-error-port) "open-module: too many arguments ~A\n" rest)
      #f)
     (else
      (let ((db (with-input-from-file
		    (car args)
		  (lambda () (read)))))
	(cond
	 ((list? db) (cons name db))
	 (else
	  (format (current-error-port) "open-module: ~A: invalid format\n"
		  (car args))
	  #f)))))))

(define (descr dbh)
  (db:get dbh descr))

(define (info dbh)
  (db:get dbh info))

(define (define-word dbh word)
  (let ((res (mapcan (lambda (elt)
		       (and (string-ci=? word (car elt))
			    elt))
		     (db:get dbh corpus))))
    (and res (cons #t res))))

(define (match-exact dbh strat word)
  (mapcan (lambda (elt)
	    (and (string-ci=? word (car elt))
		 (car elt)))
	  (db:get dbh corpus)))

(define (match-prefix dbh strat word)
  (mapcan (lambda (elt)
	    (and (string-prefix-ci? word (car elt))
		 (car elt)))
	  (db:get dbh corpus)))

(define (match-suffix dbh strat word)
  (mapcan (lambda (elt)
	    (and (string-suffix-ci? word (car elt))
		 (car elt)))
	  (db:get dbh corpus)))

(define (match-selector dbh strat word)
  (mapcan (lambda (elt)
	    (and (dico-strat-select? strat (car elt) word)
		 (car elt)))
	  (db:get dbh corpus)))

(define strategy-list
  (list (cons "exact"  match-exact)
	(cons "prefix"  match-prefix)
	(cons "suffix"  match-suffix)))

(define match-default match-prefix)

(define (match-word dbh strat word)
  (let ((sp (assoc (dico-strat-name strat) strategy-list)))
    (let ((res (cond
		(sp
		 ((cdr sp) dbh strat word))
		((dico-strat-selector? strat)
		 (match-selector dbh strat word))
		(else
		 (match-default dbh strat word)))))
      (if res
	  (cons #f res)
	  #f))))

(define (output rh n)
  (if (car rh)
      ;; Result comes from DEFINE command
      (let ((res (list-ref (cdr rh) n)))
	(display (car res))
	(newline)
	(display (cdr res)))
      ;; Result comes from MATCH command
      (display (list-ref (cdr rh) n))))

(define (result-count rh)
  (length (cdr rh)))

(define-public (example-init arg)
  (list (cons "open" open-module)
        (cons "descr" descr)
        (cons "info" info)
        (cons "define" define-word)
        (cons "match" match-word)
        (cons "output" output)
        (cons "result-count" result-count)))

;;
(dico-register-strat "suffix" "Match word suffixes")
