.. Cozenage documentation master file, created by
   sphinx-quickstart on Fri Oct 17 11:11:24 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Cozenage documentation
======================

**Cozenage** started as a 'toy' Lisp, but I am steadily, if slowly, working towards a full R5RS/R7RS
`Scheme implementation <https://standards.scheme.org/>`_.

After the bulk of the R7RS standard is implemented, I plan to add several
``(cozenage foo)`` libraries to interface with the OS, and eventually, I would like to implement some
kind of 'shell mode' which would act like an interpretive shell with Scheme syntax, kicking all
'unbound symbols' down a level as potential shell commands.

This is an educational process for me. I started writing Cozenage to enhance my understanding of
Scheme, C , and programming language fundamentals in general. Not everything here is implemented in
the most efficient or best way. Cozenage is a work in progress, and as I learn new and better
techniques I will come back and improve sections of this program. Perhaps the most conspicuous
deficiency is the lexer/parser. I am currently working through Robert Nystrom's
`"Crafting Interpreters"
<https://craftinginterpreters.com/>`_, and I will rewrite Cozenage's parser when I finish. At that
time I will also have to revisit my decision to implement the bare-bones 'AST' as a plain C array of
Cell objects, rather than as a 'proper list', as it is done in all dogmatic Lisp and Scheme variants.

.. note::

   This project is under active development. The documentation, and the code itself changes
   everyday. Be sure to pull from Github and build often to get the latest version.

This documentation is organized into three main types. **Howto**s are goal-oriented documentation.
They have specific goals, and demonstrate how to achieve that goal. **Tutorial**s are
knowledge-oriented documentation. They don't have specific goals other than to introduce and
demonstrate various concepts to work towards building general knowledge. The **Reference** is a
single document which exhaustively details all the types, procedures, syntax, special forms,
libraries, and even the underlying C internals of Cozenage. If you know what you are looking for, see
if there is a Howto on the topic, else search for the topic in the reference. If you have no
specific goal, and just want to learn more about Scheme and/or Cozenage, try a tutorial.


.. toctree::
   :maxdepth: 1
   :caption: Contents:

   howto/installation
   tutorial/get_started_with_scheme
   reference/the_cozenage_reference

